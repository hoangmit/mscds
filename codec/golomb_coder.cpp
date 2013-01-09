#include "golomb_coder.h"
#include "deltacoder.h"
#include "bitarray/bitop.h"
#include <cmath>

namespace coder {
	using namespace std;
	using namespace mscds;
	RiceCoder::RiceCoder() {}

	RiceCoder::RiceCoder(LenTp bitwidth) { this->bitwidth = bitwidth; prepare_mask(); }

	void RiceCoder::estimate(uint64_t n, uint64_t m){
		double v = -log(2.0)/ std::log((n-m)*1.0/n);
		bitwidth = (LenTp) floor((log(v) / log(2.0)) + 0.5);
		prepare_mask();
	}
		
	LenTp RiceCoder::encodelen(CodeTp n) {
		return (n >> bitwidth) + bitwidth + 1;
	}

	void RiceCoder::prepare_mask() {
		mask = (1ull << bitwidth) - 1;
	}

	std::pair<CodeTp, LenTp> RiceCoder::encode(NumTp n) {
		LenTp q = (LenTp) (n >> bitwidth);
		if (q >= sizeof(NumTp)*8) throw runtime_error("golomb code overflow");
		return CodePr(((1ull << q) - 1) | ((n & mask) << (q + 1)), q + bitwidth + 1);
	}

	std::pair<NumTp, LenTp> RiceCoder::decode2(CodeTp n) {
		LenTp q = lsb_intr(~n);
		assert(q < sizeof(NumTp)*8);
		return CodePr(((n >> (q+1)) & mask) | (q << bitwidth) , q + bitwidth + 1);
	}

	//---------------------------------------------------------------

	GolombCoder::GolombCoder() {}
	GolombCoder::GolombCoder(NumTp _M) {set_M(_M);}

	void GolombCoder::estimate(uint64_t sum, uint64_t count) {
		double v = -log(2.0)/ std::log(sum*1.0/(sum+count));
		M = (NumTp) (v + 0.5);
		if (M == 0) M = 1;
		set_M(M);
	}
	
	LenTp GolombCoder::encodelen(CodeTp n) {
		NumTp r = n % M;
		return n / M + bw + (r < cutoff ? 0 : 1);
	}

	CodePr GolombCoder::encode(NumTp n) {
		NumTp q = n / M;
		NumTp r = n % M;
		if (q >= sizeof(NumTp)*8) throw runtime_error("golomb code overflow");
		if (r < cutoff) return CodePr(((1ull << q) - 1) | (r << (q + 1)), bw + q);
		else {
			NumTp rp = (r + cutoff);
			return CodePr(((1ull << q) - 1) | ((rp >> 1) << (q + 1)) | ((rp & 1) << (bw + q)), bw + q + 1);
		}
	}

	void GolombCoder::set_M(NumTp M) {
		this->M = M;
		assert(M > 0);
		bw = ceillog2(M);
		mask = (1ull << (bw-1)) - 1;
		cutoff = (1ull << bw) - M;
	}

	CodePr GolombCoder::decode2(CodeTp n) {
		LenTp q = lsb_intr(~n);
		assert(q < sizeof(NumTp)*8);
		NumTp r = (n >> (q + 1)) & mask;
		if (r < cutoff) return CodePr(q * M + r, q + bw);
		else return CodePr(q * M + ((r << 1) | ((n >> (q + bw)) & 1)) - cutoff, q + bw + 1);
	}

	//---------------------------------------------------------------
	AdpGolombCoder::AdpGolombCoder(): udtcycle(400)  {reset();}
	AdpGolombCoder::AdpGolombCoder(unsigned int updatecycle): udtcycle(updatecycle), i(0), sum(0), count(0), lsum(0), lcount(0) {
		reset();
	}

	void AdpGolombCoder::reset() {
		sum = 0;
		count = 0;
		lsum = 0;
		lcount = 0;
		i = 3*udtcycle/4;
		set_M(1);
	}

	void AdpGolombCoder::estimate(uint64_t sum, uint64_t count) {
		double v = -log(2.0)/ std::log(sum*1.0/(sum+count));
		M = (NumTp) (v + 0.5);
		if (M == 0) M = 1;
		set_M(M);
	}

	void AdpGolombCoder::set_M(NumTp M) {
		this->M = M;
		assert(M > 0);
		bw = ceillog2(M);
		mask = (1ull << (bw-1)) - 1;
		cutoff = (1ull << bw) - M;
	}

	bool AdpGolombCoder::update(NumTp n) {
		i++;
		lsum += n;
		lcount++;
		if (i >= udtcycle) {
			sum = lsum + (sum + 1) / 2;
			count = lcount + (count + 1) / 2;
			lsum = 0;
			lcount = 0;
			i = 0;
			estimate(sum, count);
			return true;
		}else return false;
	}

	CodePr AdpGolombCoder::encode_s(NumTp n) {
		NumTp q = n / M;
		NumTp r = n % M;
		if (q >= 23) {
			return concatc(CodePr(1ull << 23, 24), DeltaCoder::encode(n - M * 23 + 1));
		}else
			if (r < cutoff) return CodePr((1ull << q) | (r << (q + 1)), bw + q);
			else {
				NumTp rp = (r + cutoff);
				return CodePr((1ull << q) | ((rp >> 1) << (q + 1)) | ((rp & 1) << (bw + q)), bw + q + 1);
			}
	}

	CodePr AdpGolombCoder::decode_s(CodeTp n) {
		LenTp q = lsb_intr(n);
		if (q == 23) {
			CodePr r = DeltaCoder::decode2(n >> 24);
			return CodePr(r.first + M * 23 - 1, 24 + r.second);
		}
		NumTp r = (n >> (q + 1)) & mask;
		if (r < cutoff) return CodePr(q * M + r, q + bw);
		else return CodePr(q * M + ((r << 1) | ((n >> (q + bw)) & 1)) - cutoff, q + bw + 1);
	}

	CodePr AdpGolombCoder::encode(NumTp n) {
		CodePr ret = encode_s(n);
		update(n);
		return ret;
	}
	
	CodePr AdpGolombCoder::decode(CodeTp n) {
		CodePr ret = decode_s(n);
		update(ret.first);
		return ret;
	}
}

