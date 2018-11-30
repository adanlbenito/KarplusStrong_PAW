#include <KarplusStrong.h>
#include <math.h>
#include <stdio.h>


KarplusStrong::KarplusStrong() {}

KarplusStrong::KarplusStrong(float fs, float minFrequency, float initialFrequency)
{
	setup(fs, minFrequency, initialFrequency);
}

int KarplusStrong::setup(float fs, float minFrequency, float initialFrequency)
{
	fs_ = fs;
	unsigned int bufferLength = 1 + (unsigned int)(fs_ / minFrequency + 0.5f);
	delayBuffer.resize(bufferLength, 0);
	setFrequency(initialFrequency);

	return 0;
}

float KarplusStrong::process(float input) 
{	
	updateReadPointer();
	
	float prev = interpolatedRead(readPointer - 1.f);
	
	// Difference equation for K-S (including input excitation):
	// y(n) = scaling * x(n) + damping * (y(n-N) + y(n-(N+1)) / 2
	float out = input + dampingFactor_ * ( interpolatedRead(readPointer) + prev ) / 2.0f;
	delayBuffer[writePointer] = out;
	
	updateWritePointer();

	return out;
}

void KarplusStrong::process(float* input, float* output, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++)
		output[i] = process(input[i]);
}

void KarplusStrong::setFrequency(float frequency)
{
	delayLength_ = fs_/frequency; // Real value for the period of the first partial
}

void KarplusStrong::setDamping(float damping)
{
	dampingFactor_ = damping;
}

void KarplusStrong::updateReadPointer()
{
	readPointer = (writePointer - delayLength_ + delayBuffer.size());
	while(readPointer >= delayBuffer.size())
		readPointer -= delayBuffer.size();
}
float KarplusStrong::interpolatedRead(float index)
{
	while(index < 0)
		index += delayBuffer.size();
	int pIndex = (int)index;
	int nIndex = pIndex + 1;
	while(nIndex >= delayBuffer.size())
		nIndex -= delayBuffer.size();
	float frac = index - pIndex;
	float pVal = delayBuffer[pIndex];
	float nVal = delayBuffer[nIndex];

	return linearInterpolation(frac, pVal, nVal);
}

float KarplusStrong::linearInterpolation(float index, float pVal, float nVal)
{
	return pVal + index * (nVal - pVal);
}

void KarplusStrong::updateWritePointer()
{
	if(++writePointer >= delayBuffer.size())
		writePointer = 0;
}
KarplusStrong::~KarplusStrong() 
{
	cleanup();
}	

void KarplusStrong::cleanup()
{
}
