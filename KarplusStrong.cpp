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
	onePole.setup(1, 2);
	weight = 0.5f;
	fs_ = fs;
	unsigned int bufferLength = 2 * (1 + (unsigned int)(fs_ / minFrequency + 0.5f)); // 2*: when inverted, the frequency halves
	delayBuffer.resize(bufferLength, 0);
	setFrequency(initialFrequency);

	return 0;
}

float KarplusStrong::process(float input) 
{	
	float readPointer = (writePointer - delayLength_ * (invert ? 2 : 1) + delayBuffer.size());
	while(readPointer >= delayBuffer.size())
		readPointer -= delayBuffer.size();
	float prev = interpolatedRead(readPointer - 1.f);
	
	// Difference equation for K-S (including input excitation):
	float in = interpolatedRead(readPointer);
	// add a one-pole lowpass after the average filter + attenuation on the delay line:
	// y(n) = scaling * x(n) + lossFactor * (y(n-N) + y(n-(N+1)))/2 * alpha + y(n - 1) * (1 - alpha)
	float filterIn = weight * in + prev * (1.f - weight);
	float filterOut = onePole.process(filterIn);

	float out = input + (invert ? 1 : -1) * lossFactor_ * filterOut;
	delayBuffer[writePointer] = out;

	if(++writePointer >= delayBuffer.size())
		writePointer = 0;
	return out;
}

void KarplusStrong::process(float* input, float* output, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++)
		output[i] = process(input[i]);
}

void KarplusStrong::setWeight(float weight)
{
	this->weight = weight;
}

void KarplusStrong::setInvert(bool invert)
{
	this->invert = invert;
}

void KarplusStrong::setFrequency(float frequency)
{
	delayLength_ = fs_/frequency; // Real value for the period of the first partial
}

void KarplusStrong::setLossFactor(float lossFactor)
{
	lossFactor_ = lossFactor;
}

void KarplusStrong::setDamping(float damping)
{
	// lossFactor_ = damping * 0.2f + 0.792f; // some hard-coded values to avoid pitch change as damping changes. Better check out the textbook!
	onePole.setFilter(damping);
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

KarplusStrong::~KarplusStrong() 
{
	cleanup();
}	

void KarplusStrong::cleanup()
{
}
