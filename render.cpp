#include <Bela.h>
#include <KarplusStrong.h>
#include <Scope/Scope.h>
#include <math.h>

KarplusStrong string;
Scope scope;

float gGain = 0.55;
int gAudioFramesPerAnalogFrame = 0;

float gFreqRange[2] = { 261.63, 523.25 };
float gDampingRange[2] = { 0.86, 0.998 };

int gFsrInput = 0;
int gPotInput = 1;

float gAnalogFullScale = 3.3/4.096;
float gFsrRange[2] = { 0.4, gAnalogFullScale };

float logMap(float input, float inRange0, float inRange1, float outRange0, float outRange1)
{
	float base = powf(10, outRange0);
	float range = powf(10, outRange1) - base;
	float normIn = map(input, inRange0, inRange1, 0, 1);
	float out = log10(base + normIn * range);
	return out;
}

bool setup(BelaContext *context, void *userData)
{
	if(string.setup(2, context->audioSampleRate) != 0) {
		fprintf(stderr, "Unable to initialise touch sensor\n");
		return false;
	}
	
	string.setDamping(0.989);
	
	scope.setup(3, context->audioSampleRate);
	
	gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

	return true;
}

void render(BelaContext *context, void *userData)
{
	float ksOut;
	float damping, frequency;
	float fsrVal, potVal;
	
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		
		ksOut = string.process(audioRead(context, n, 0));
		
		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, gGain*ksOut);
			
			if(gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
				// read analog inputs and update frequency and damping
				// Depending on the sampling rate of the analog inputs, this will
				//	happen every audio frame (if it is 44100)
				//	or every two audio frames (if it is 22050)
				
				fsrVal = analogRead(context, n/gAudioFramesPerAnalogFrame, gFsrInput);
				fsrVal = constrain(fsrVal, gFsrRange[0], gFsrRange[1]);
				damping = logMap(fsrVal, gFsrRange[1], gFsrRange[0], gDampingRange[0], gDampingRange[1]);
				potVal = analogRead(context, n/gAudioFramesPerAnalogFrame, gPotInput);
				frequency = map(potVal, 0, 1, gFreqRange[0], gFreqRange[1]);
				
				string.setFrequency(frequency);
				string.setDamping(damping);
			}
			scope.log(fsrVal, potVal, damping);
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
	string.cleanup();
}