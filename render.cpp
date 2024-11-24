#include <Bela.h>
#include <KarplusStrong.h>
#include <libraries/Scope/Scope.h>
#include <math.h>
#include <libraries/math_neon/math_neon.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>

KarplusStrong gPiezoString;
KarplusStrong gMicString;
Scope gScope;
Gui gui;
GuiController controller;
float gOutputGain = 0.55;
float gFreqRatio = 1.333;

float gFreqRange[2] = { 130.8165 / 2, 523.25 };
float gLossFactorRange[2] = { 0.9, 0.994 };
float gDampingRange[2] = { 0.01, 1 };
unsigned int gPiezoChannel = 0;
unsigned int gMicChannel = 1;
int gFsrChannel = 0;
int gPotChannel = 1;
float gAnalogFullScale = 3.3/4.096;
float gFsrRange[2] = { 0.4, gAnalogFullScale };

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
#define KSSTRING
// #define KSDRUM
#ifdef KSSTRING
unsigned int gDampSliderIdx;
unsigned int gLossFactorSliderIdx;
unsigned int gInvertSliderIdx;
unsigned int gWeightSliderIdx;
unsigned int gFreqSliderIdx;
#endif // KSSTRING

#ifdef KSDRUM
#include "KsDrum.h"
KsDrum drum;
unsigned int gPSliderIdx;
unsigned int gBSliderIdx;
size_t drumSize = 4096;
#endif // KSDRUM

bool setup(BelaContext *context, void *userData)
{
	gui.setup(context->projectName);
	controller.setup(&gui, "controls");
#ifdef KSSTRING
	gDampSliderIdx = controller.addSlider("damp", 1);
	gLossFactorSliderIdx = controller.addSlider("loss", 0.95);
	gInvertSliderIdx = controller.addSlider("invert", 0, 0, 1, 1);
	gWeightSliderIdx = controller.addSlider("weight", 0.5, 0.5, 1.3);
	gFreqSliderIdx = controller.addSlider("freq", 0.5, 0, 1);
#endif // KSSTRING
#ifdef KSDRUM
	gPSliderIdx = controller.addSlider("p", 400, 1, drumSize, 1);
	gBSliderIdx = controller.addSlider("b", 0.5, 0, 1, 0.001);
	drum.setup(drumSize);
#endif // KSDRUM
	gPiezoString.setup(context->audioSampleRate, gFreqRange[0], 432);
	gMicString.setup(context->audioSampleRate, gFreqRange[0], 432.f * gFreqRatio);
	gScope.setup(2, context->audioSampleRate);
	return true;
}

void render(BelaContext *context, void *userData)
{
	static int count = 0;
	count += context->audioFrames;
	if(count % 40000 >= 40000 - context->audioFrames)
	{
		count = 0;
	}
#ifdef KSSTRING
	float lossFactor = logMap(controller.getSliderValue(gLossFactorSliderIdx), 0, 1, gLossFactorRange[0], gLossFactorRange[1]);
	gPiezoString.setLossFactor(lossFactor);
	gMicString.setLossFactor(lossFactor);

	bool invert = controller.getSliderValue(gInvertSliderIdx);
	gPiezoString.setInvert(invert);
	gMicString.setInvert(invert);

	float damping = logMap(controller.getSliderValue(gDampSliderIdx), 0, 1, gDampingRange[0], gDampingRange[1]);
	gPiezoString.setDamping(damping);
	gMicString.setDamping(damping);

	float weight = controller.getSliderValue(gWeightSliderIdx);
	gPiezoString.setWeight(weight);
	gMicString.setWeight(weight);

	float frequency = logMap(controller.getSliderValue(gFreqSliderIdx), 0, 1, gFreqRange[0], gFreqRange[1]);

#endif // KSSTRING
#ifdef KSDRUM
	float p = controller.getSliderValue(gPSliderIdx);
	float b = controller.getSliderValue(gBSliderIdx);
#endif // KSDRUM
	for(unsigned int n = 0; n < context->audioFrames; n++) {
#ifdef KSSTRING
		gPiezoString.setFrequency(frequency);
		gMicString.setFrequency(frequency * gFreqRatio);
		float piezoInput = audioRead(context, n, gPiezoChannel);
		float micInput = audioRead(context, n, gMicChannel);
		piezoInput = 0;
		micInput = 0;
		if(count < 100)
		{
			// trigger piezoInput
			srand(0); // make sure they all sound the same
			float in = rand() / float(RAND_MAX) * 2.f - 1.f;
			piezoInput = in;
		}
		float piezoStringOut = gPiezoString.process(piezoInput);
		float micStringOut = gMicString.process(micInput);
		float out = gOutputGain * (piezoStringOut + micStringOut);
#endif // KSSTRING
#ifdef KSDRUM
		// ks drum
		if(count == 0)
			drum.setup(drumSize);
		float r = rand() / float(RAND_MAX); // could get rand from an analog input, e.g.: fmodf(analogRead(context, 0, n/2) * 1000.f, 1);
		float out = drum.process(r, b, p);
#endif /// KSDRUM
		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, out);
		}
		gScope.log(out);
	}
}

void cleanup(BelaContext *context, void *userData)
{}
