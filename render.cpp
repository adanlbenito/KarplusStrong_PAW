#include <Bela.h>
#include <KarplusStrong.h>
#include <Scope.h>
#include <math.h>
#include <math_neon.h>

KarplusStrong gPiezoString;
KarplusStrong gMicString;
Scope gScope;

float gOutputGain = 0.55;
float gFreqRatio = 1.333;

float gFreqRange[2] = { 130.8165, 523.25 };
float gDampingRange[2] = { 0.86, 0.992 };
unsigned int gPiezoChannel = 0;
unsigned int gMicChannel = 1;
int gFsrChannel = 0;
int gPotChannel = 1;
float gAnalogFullScale = 3.3/4.096;
float gFsrRange[2] = { 0.4, gAnalogFullScale };

int gAudioFramesPerAnalogFrame;

float logMap(float input, float inRange0, float inRange1, float outRange0, float outRange1)
{
	float base = powf_neon(10, outRange0);
	float range = powf_neon(10, outRange1) - base;
	float normIn = map(input, inRange0, inRange1, 0, 1);
	float out = log10f_neon(base + normIn * range);
	return out;
}

bool setup(BelaContext *context, void *userData)
{
	gPiezoString.setup(context->audioSampleRate, gFreqRange[0], 432);
	gMicString.setup(context->audioSampleRate, gFreqRange[0], 432.f * gFreqRatio);

	gScope.setup(4, context->audioSampleRate);
	
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

	return true;
}

void render(BelaContext *context, void *userData)
{
	float damping, frequency;
	float fsrVal, potVal;
	
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		
		if(gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
			// read analog inputs and update frequency and damping
			// Depending on the sampling rate of the analog inputs, this will
			//	happen every audio frame (if it is 44100)
			//	or every two audio frames (if it is 22050)

			fsrVal = analogRead(context, n/gAudioFramesPerAnalogFrame, gFsrChannel);
			fsrVal = constrain(fsrVal, gFsrRange[0], gFsrRange[1]);
			damping = logMap(fsrVal, gFsrRange[1], gFsrRange[0], gDampingRange[0], gDampingRange[1]);
			potVal = analogRead(context, n/gAudioFramesPerAnalogFrame, gPotChannel);
			frequency = map(potVal, 0, 1, gFreqRange[0], gFreqRange[1]);

			gPiezoString.setFrequency(frequency);
			gPiezoString.setDamping(damping);
			gMicString.setFrequency(frequency * gFreqRatio);
			gMicString.setDamping(damping);
		}

		float piezoInput = audioRead(context, n, gPiezoChannel);
		float micInput = audioRead(context, n, gMicChannel);
		float piezoStringOut = gPiezoString.process(piezoInput);
		float micStringOut = gMicString.process(micInput);

		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, gOutputGain * (piezoStringOut + micStringOut));
		}
		gScope.log(micInput, micStringOut, fsrVal, potVal);
	}
}

void cleanup(BelaContext *context, void *userData)
{}