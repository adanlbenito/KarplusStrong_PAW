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
float gLossFactorRange[2] = { 0.86, 0.992 };
float gDampingRange[2] = { 0.1, 1 };
unsigned int gPiezoChannel = 0;
unsigned int gMicChannel = 1;
int gFsrChannel = 0;
int gPotChannel = 1;
float gAnalogInputRange = 3.3/4.096;
float gFsrRange[2] = { 0.4, gAnalogInputRange };

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

	gScope.setup(2, context->audioSampleRate);
	
	if(context->audioFrames != context->analogFrames)
	{
		fprintf(stderr, "This example requires the same number of analog and audio frames\n"); // for simplicity
		return false;
	}
	return true;
}

void render(BelaContext *context, void *userData)
{
	
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		
		float piezoInput = audioRead(context, n, gPiezoChannel);
		float piezoStringOut = gPiezoString.process(piezoInput);

		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, gOutputGain * piezoStringOut);
		}
		gScope.log(piezoInput, piezoStringOut);
	}
}

void cleanup(BelaContext *context, void *userData)
{}
