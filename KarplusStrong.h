#include <vector>

class KarplusStrong
{

	private:
		std::vector<float> delayBuffer;

		float readPointer;
		int writePointer = 0;

		// Sample rate
		float fs_;

		// Karplus-strong coefficients
		float delayLength_;
		float dampingFactor_ = 0.989;

		void updateReadPointer();

		void updateWritePointer();

		float interpolatedRead(float index);

	public:
		KarplusStrong();
		KarplusStrong(float fs, float minFrequency, float initialFrequency);
		~KarplusStrong();

		int setup(float fs, float minFrequency, float initialFrequency);
		void cleanup();
		
		float process(float input);

		void process(float* input, float* output, unsigned int length);
		
		void setFrequency(float frequency);

		void setDamping(float damping);

		static float linearInterpolation(float index, float pVal, float nVal);
};
