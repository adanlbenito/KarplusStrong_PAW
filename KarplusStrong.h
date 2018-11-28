#include <vector>

KarplusStrong
{

	private:
		std::vector delayBuffer;

		int bufferLength;
		int readPointer;
		int writePointer = 0;

		struct allpassCoefficients {
			float apC;
			float apYm1;
			float apXm1;
		};

		struct tuningParameters {
			float p1; // Real value for the period of the first partial
			float pcF1; // Low-frequency phase delay of 1st order all-pass
			float epsilon = 0.0001;
		};

		int fs_;
	public:
		KarplusStrong();
		~KarplusStrong();

		int setup(unsigned int length, unsigned int fs, float frequency = 440.0);
		void cleanup();

		struct ksCoefficients {
			float frequency;
			int delayLength;
			float dampingFactor;
		};
		
		float process();
		void process(float* input, float* output, unsigned int length);
		
		void updateFrequency(float frequency);
		void updateReadPointer();
		void updateWritePointer();

}
