#include <KarplusStrong.h>


KarplusStrong::KarplusStrong() {}

KarplusStrong::KarplusStrong(unsigned int length, unsigned int fs, float frequency)
{
	setup(length, fs, frequency);
}

int KarplusStrong::setup(unsigned int length, unsigned int fs, float frequency)
{
	fs_ = fs;
	bufferLength = length * fs_;
	delayBuffer.resize(bufferLength);
	//fill(delayBuffer.begin(), delayBuffer.end(), 0.0);
	setFrequency(frequency);

	return 0;
}

float KarplusStrong::process(float input) 
{	
	float prev;
	float outPt;
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
	// y(n) = scaling * x(n) + damping * (y(n-N) + y(n-(N+1)) / 2
	float scalingFactor = dampingFactor_/2.0f;
	outPt = scalingFactor * input + dampingFactor_ * ( delayBuffer[readPointer] + prev ) / 2.0f;
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
	delayLength_ = floor(p1 - 0.5 - epsilon);
	pcF1 = p1 - delayLength_ - 0.5;
	apC = (1 - pcF1)/(1 + pcF1);
}


void KarplusStrong::setDamping(float damping)
{
	dampingFactor_ = damping;
}

void KarplusStrong::updateReadPointer()
{
	readPointer = (writePointer - delayLength_ + bufferLength) % bufferLength;
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
