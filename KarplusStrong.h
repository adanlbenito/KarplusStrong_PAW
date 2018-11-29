#include <vector>

class KarplusStrong
{

	private:
		std::vector<float> delayBuffer;

		int bufferLength;
		float readPointer;
		int writePointer = 0;

		// All-pass coefficients
		float apC;
		float apYm1;
		float apXm1;

		// Tuning parameters
		float p1; // Real value for the period of the first partial
		float pcF1; // Low-frequency phase delay of 1st order all-pass
		float epsilon = 0.0001;

		int fs_;

		// Karplus-strong coefficients
		float delayLength_;
		float dampingFactor_ = 0.989;

		void updateReadPointer();
		void updateWritePointer();
		float interpolatedRead(float index);

	public:
		KarplusStrong();
		KarplusStrong(unsigned int length, unsigned int fs, float frequency);
		~KarplusStrong();

		int setup(unsigned int length, unsigned int fs, float frequency = 440.0);
		void cleanup();
		
		float tuningFilter(float input);
		float process(float input);
		void process(float* input, float* output, unsigned int length);
		
		void setFrequency(float frequency);
		void setDamping(float damping);

		static float linearInterpolation(float index, float pVal, float nVal);
};
