#include <vector>
class KsDrum
{
public:
	// See Karplus and Strong, CMJ, 1983
	// Also, see:
	// https://ccrma.stanford.edu/~sdill/220A-project/drum-ks.txt
	/**
	 * Initialise the drum
	 *
	 * @param maxLen the maximum length of the wavetable
	 */
	void setup(size_t maxLen)
	{
		table.resize(maxLen);
		for(auto& v : table)
			v = (2.f * rand() / float(RAND_MAX)) - 1.f;
		n = 0;
	}
	/**
	 *
	 * Get the next output.
	 *
	 * @param rand a random value
	 * @param b the blend factor, can range from 0 to 1. `b = 1/2` introduces the most
	 * randomness and produces the best "snare" sounds.
	 * `b` near `0` simply averages the samples, and produces string-like sounds where `p` controls
	 * the pitch.
	 * `b` near 1 produces wierd electric crash cymbal-like sounds where most of the pitches
	 * die out quickly. `b = 1` doesn't work for constant or sine wavetables.
	 * @param p the wavetable length. It should be smaller or equal to the value passed
	 * to setup() and it affects the decay rate of the sound (big p = long decay) as well
	 * as the pitch somewhat (big p = low pitch). `p` should be in a range from about 150 to 500.
	 */
	float process(float rnd, float b, size_t p)
	{
		size_t idx0 = (n - p + table.size()) % table.size();
		size_t idx1 = (idx0 - 1 + table.size()) % table.size();
		float out = ((rnd < b) - 0.5f) * (table[idx1] + table[idx0]);
		table[n] = out;
		n++;
		if(n >= table.size())
			n -= table.size();
		return out;
	}
private:
	std::vector<float> table;
	std::vector<float> delay;
	size_t n = 0;
};
