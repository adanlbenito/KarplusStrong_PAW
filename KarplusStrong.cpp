#include <KarplusStrong.h>


KarplusStrong::KarplusStrong() {}

int KarplusStrong::setup(unsigned int length, unsigned int fs)
{
	bufferLength = length * fs;
	delayBuffer.resize(bufferLength);
}

float KarplusStrong::process(float input) 
{	
	float prev;
	float outPt
	float out;

	updateReadPointer();
	
	if(readPointer == 0)
	{
		prev = delayBuffer[bufferLength - 1];
	}
	else
	{
		prev = delayBuffer[readPointer - 1];
	}
	
	// Difference equation for K-S (modified to include input excitation):
	// y(n) = scalingi * x(n) + damping * (y(n-N) + y(n-(N+1)) / 2
	float scalingFactor = dampingFactor/2.0f;
	outPt = scalingFactor * input + dampingFactor * ( delayBuffer[readPointer] + prev ) / 2.0f;
	// Difference equation for all-pass filter used to correct tuning errors:
	// y(n) = C * x(n) - x(n-1) + C * y(n-1) 
	out = apC * (outPt + apYm1) + apXm1;

	apYm1 = out;
	apXm1 = outPt;
	
	delayBuffer[writePointer] = out;
	
	updateWritePointer();

	return out;
}

void KarplusStrong::process(float* input, float* output, unsigned int length)
{
}

void KarplusStrong::updateReadPointer()
{
	readPointer = (writePointer - delayLength + bufferLength) % bufferLength;
}	

void KarplusStrong::updateWritePointer()
{
	if(++writePointer >= bufferLength) 
		writePointer = 0;
}

KarplusStrong::~KarplusStrong() 
{
	cleanup();
}	

void KarplusStrong::cleanup()
{
}
