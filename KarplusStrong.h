#include <vector>
#include <stddef.h>
#include <libraries/OnePole/OnePole.h>

class KarplusStrong
{

	private:
		std::vector<float> delayBuffer;

		size_t writePointer = 0;

		// Sample rate
		float fs_;

		// Karplus-strong coefficients
		float delayLength_;
		float lossFactor_ = 0.989;
		float weight = 0.5f;
		OnePole onePole;
		float interpolatedRead(float index);
		bool invert;

	public:
		KarplusStrong();
		KarplusStrong(float fs, float minFrequency, float initialFrequency);
		~KarplusStrong();

		int setup(float fs, float minFrequency, float initialFrequency);
		void cleanup();
		
		float process(float input);

		void process(float* input, float* output, unsigned int length);
		
		/**
		 * Invert the signal before writing it back into the delay line.
		 * An automatic frequency compensation is applied when this is enabled.
		 */
		void setInvert(bool invert);

		/**
		 * Resulting sound may be slightly off from the nominal value,
		 * as there's no phase compensation in place.
		 */
		void setFrequency(float frequency);

		/**
		 * The output is the linear combination of two values from the delay line.
		 * In the original algorithm, these are averaged. Here you can set their weights:
		 * one will be scaled by @p weight, the other by `1 - weight`. Stable values are
		 * between 0 and slightly more than 1.
		 */
		void setWeight(float weight);
		void setLossFactor(float lossFactor);
		void setDamping(float damping);

		static float linearInterpolation(float index, float pVal, float nVal);
};
