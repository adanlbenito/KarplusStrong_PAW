#include <Bela.h>
#include <KarplusStrong.h>
#include <Scope/Scope.h>

KarplusStrong string;
Scope scope;

bool setup(BelaContext *context, void *userData)
{
	if(string.setup(2, context->audioSampleRate) != 0) {
		fprintf(stderr, "Unable to initialise touch sensor\n");
		return false;
	}
	
	string.setDamping(0.989);
	
	scope.setup(2, context->audioSampleRate);
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	float ksOut;
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		
		ksOut = string.process(audioRead(context, n, 0));
		
		for(unsigned int ch = 0; ch < context->audioOutChannels; ch++){
			audioWrite(context, n, ch, ksOut);
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
	string.cleanup();
}