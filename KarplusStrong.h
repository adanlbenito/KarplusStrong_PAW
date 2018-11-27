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

	public:
		KarplusStrong();
		~KarplusStrong();
		
		int setup(unsigned int length, unsigned int fs);
		void cleanup();

		struct ksCoefficients {
			float frequency;
			int delayLength;
			float dampingFactor;
		};
		
		float process();
		void process(float* input, float* output, unsigned int length);
		
		void updateReadPointer();
		void updateWritePointer();

}
