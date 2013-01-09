#pragma once

#include <cassert>
#include "coder.h"

namespace coder {
	// little endian style

	class RiceCoder: public Coder {
	public:
		RiceCoder();
		RiceCoder(LenTp bitwidth);
		/** m numbers and total sum is n (good when m < 0.9 n)  */
		void estimate(uint64_t n, uint64_t m);
		
		LenTp encodelen(CodeTp n);
		CodePr encode(NumTp n);
		CodePr decode2(CodeTp n);
		LenTp get_bitwidth() { return bitwidth; }
	private:
		void prepare_mask();
		LenTp bitwidth;
		NumTp mask;
	};

	class GolombCoder: public Coder {
	public:
		GolombCoder();
		GolombCoder(NumTp M);
		/** m numbers and total sum is n (good when m < 0.5 n) */
		void estimate(uint64_t sum, uint64_t count);
		LenTp encodelen(CodeTp n);
		CodePr encode(NumTp n);
		CodePr decode2(CodeTp n);
		NumTp get_M() { return M; }
		void set_M(NumTp M);
	private:
		NumTp M, cutoff, mask;
		LenTp bw;
	};

	class AdpGolombCoder {
	public:
		AdpGolombCoder();
		AdpGolombCoder(unsigned int updatecycle);
		// must encode or decode sequencial
		CodePr encode_s(NumTp n);
		CodePr decode_s(CodeTp n);
		CodePr encode(NumTp n);
		CodePr decode(CodeTp n);
		void reset();
		bool update(NumTp n);
		NumTp current_M() { return M; }
		void set_M(NumTp M);
	private:
		NumTp M, cutoff, mask;
		LenTp bw;

		uint64_t sum, lsum;
		size_t count, lcount;

		size_t udtcycle, i;
		void estimate(uint64_t sum, uint64_t count);
		LenTp encodelen(CodeTp n);
		
	};
}
