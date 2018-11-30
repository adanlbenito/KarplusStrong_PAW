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
	float prev;
	float outPt;
	float out;

	updateReadPointer();
	
	prev = interpolatedRead(readPointer - 1.f);
	
	// Difference equation for K-S (modified to include input excitation):
	// y(n) = scaling * x(n) + damping * (y(n-N) + y(n-(N+1)) / 2
	float scalingFactor = dampingFactor_ / 2.0f; //TODO: remove this
	outPt = scalingFactor * input + dampingFactor_ * ( interpolatedRead(readPointer) + prev ) / 2.0f;
	out = tuningFilter(outPt);
	delayBuffer[writePointer] = out;
	
	updateWritePointer();

	return out;
}

float KarplusStrong::tuningFilter(float input)
{
	// Difference equation for all-pass filter used to correct tuning errors:
	// y(n) = C * x(n) + x(n-1) - C * y(n-1) 
	float out = apC * (input - apYm1) + apXm1;

	apYm1 = out;
	apXm1 = input;
	
	return out;
}

void KarplusStrong::process(float* input, float* output, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++)
		output[i] = process(input[i]);
}

void KarplusStrong::setFrequency(float frequency)
{
	p1 = fs_/frequency;
	delayLength_ = p1 - 0.5f - epsilon;
	if(delayLength_ > delayBuffer.size()) // clip value if needed
		delayLength_ = delayBuffer.size() - 2; // -2 to allow for interpolation
	pcF1 = p1 - delayLength_ - 0.5f;
	apC = (1.f - pcF1)/(1.f + pcF1);

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
