#include <Bela.h>
#include <KarplusStrong.h>
#include <libraries/Scope/Scope.h>
#include <math.h>
#include <libraries/math_neon/math_neon.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Trill/Trill.h>
#include <RtMsgFifo.h>
#include "TrillMonitor.h"

RtNonRtMsgFifo gPipe;
Scope gScope;
Gui gui;
GuiController controller;
Trill craft;
TrillMonitor trillMonitor;
struct Channel {
	unsigned int c;
	float threshOnset;
	float threshDamp;
	struct {
		// set by i2c thread
		float value = 0;
		// set by audio thread
		int attack = -1;
	} s;
};

std::vector<Channel> gChannels = {
	{0, 0.60, 0.7},
	{2, 0.5, 0.75},
	{4, 0.4, 0.75},
	{6, 0.4, 0.75},
	{8, 0.4, 0.75},
	{10, 0.4, 0.75},
	{12, 0.4, 0.75},
	{14, 0.4, 0.75},
	{16, 0.4, 0.75},
	{18, 0.4, 0.75},
};
std::vector<KarplusStrong> synths(gChannels.size());

struct OnsetMsg { 
	unsigned int channel;
	float velocity;
};

float gFreqRatio = 1.333;

float gFreqRange[2] = { 130.8165 / 2, 523.25 };
float gLossFactorRange[2] = { 0.9, 0.994 };
float gDampingRange[2] = { 0.01, 1 };

float logMap(float input, float inRange0, float inRange1, float outRange0, float outRange1)
{
	// the division/multiply by fac is to avoid reaching numerical limits
	const float fac = 100000;
	float base = powf_neon(10, outRange0 / fac);
	float range = powf_neon(10, outRange1 / fac) - base;
	float normIn = map(input, inRange0, inRange1, 0, 1);
	float out = log10f_neon(base + normIn * range) * fac;
	return out;
}

unsigned int gDampSliderIdx;
unsigned int gLossFactorSliderIdx;
unsigned int gInvertSliderIdx;
unsigned int gWeightSliderIdx;
unsigned int gFreqSliderIdx;
unsigned int gGainSliderIdx;
unsigned int gLevelSliderIdx;

void readTrill(void*)
{
	unsigned int count = 0;
	std::vector<unsigned int> lastOnset(gChannels.size()); // for crude debounce
	while(!Bela_stopRequested())
	{
		count++;
		craft.readI2C(true);
		for(size_t n = 0; n < gChannels.size(); ++n)
		{
			Channel& c = gChannels[n];
			float val = craft.rawData[gChannels[n].c];
			float pastVal = gChannels[n].s.value;
			if(pastVal < c.threshOnset && val >= c.threshOnset && count - lastOnset[n] > 5)
			{
				struct OnsetMsg msg;
				msg.channel = n;
				msg.velocity = val - gChannels[n].s.value;
				gPipe.writeNonRt(msg);
				lastOnset[n] = count;
			}
			gChannels[n].s.value = val;
		}
		trillMonitor.i2cCallback(craft);
		usleep(10000);
	}
}

bool setup(BelaContext *context, void *userData)
{
	trillMonitor.setup(context);
	gPipe.setup("onsetpipe");
	craft.setup(1, Trill::CRAFT, 0x37);
	usleep(10000);
	craft.setMode(Trill::RAW);
	usleep(10000);
	craft.setPrescaler(5);
	usleep(10000);
	// craft.updateBaseline();
	// usleep(10000);
	Bela_runAuxiliaryTask(readTrill);
	gui.setup(context->projectName);
	controller.setup(&gui, "controls");

	gDampSliderIdx = controller.addSlider("damp", 1);
	gLossFactorSliderIdx = controller.addSlider("loss", 0.95);
	gInvertSliderIdx = controller.addSlider("invert", 0, 0, 1, 1);
	gWeightSliderIdx = controller.addSlider("weight", 0.5, 0.5, 1.3);
	gFreqSliderIdx = controller.addSlider("freq", 0.5, 0, 1);
	gGainSliderIdx = controller.addSlider("gain", 0.5, 0, 1);
	gLevelSliderIdx = controller.addSlider("level", 0.5, 0, 1);

	for(auto& s : synths)
		s.setup(context->audioSampleRate, gFreqRange[0], 432);

	gScope.setup(synths.size(), context->audioSampleRate);
	return true;
}

void render(BelaContext *context, void *userData)
{
	float lossFactor = logMap(controller.getSliderValue(gLossFactorSliderIdx), 0, 1, gLossFactorRange[0], gLossFactorRange[1]);
	for(auto& s : synths)
		s.setLossFactor(lossFactor);

	bool invert = controller.getSliderValue(gInvertSliderIdx);
	for(auto& s : synths)
		s.setInvert(invert);

	float dampingSlider = controller.getSliderValue(gDampSliderIdx);
	float damping = logMap(dampingSlider, 0, 1, gDampingRange[0], gDampingRange[1]);
	for(size_t i = 0; i < synths.size(); ++i)
	{
		float d = damping * (gChannels[i].s.value > gChannels[i].threshDamp ? 0.1f : 1.f);
		synths[i].setDamping(d);
	}

	float weight = controller.getSliderValue(gWeightSliderIdx);
	for(auto& s : synths)
		s.setWeight(weight);

	float frequency = logMap(controller.getSliderValue(gFreqSliderIdx), 0, 1, gFreqRange[0], gFreqRange[1]);
	for(size_t n = 0; n < synths.size(); ++n)
		synths[n].setFrequency(frequency * (1.f + (gFreqRatio - 1.f) * n));

	float gain = logMap(controller.getSliderValue(gGainSliderIdx), 0, 1, 0.001, 10);
	float max = 0;
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		float logs[synths.size()];
		float out = 0;
		OnsetMsg msg;
		while(1 == gPipe.readRt(msg))
		{
			Channel& c = gChannels[msg.channel];
			c.s.attack = msg.velocity * msg.velocity * msg.velocity * 1000;
			rt_printf("%u %.3f %u\n", msg.channel, msg.velocity, c.s.attack);
		}
		for(size_t i = 0; i < gChannels.size(); ++i)
		{
			Channel& c = gChannels[i];
			float in = 0;
			if(c.s.attack-- >= 0)
				in = rand() / float(RAND_MAX) * 2.f - 1.f;
			float val = synths[i].process(in);
			logs[i] = val;
			out += val;
		}
		out *= gain;
		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, out);
		}
		max = std::max(max, std::abs(out));
		gScope.log(logs);
	}
}

void cleanup(BelaContext *context, void *userData)
{}
