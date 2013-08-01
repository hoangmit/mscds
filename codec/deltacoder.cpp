

#include "deltacoder.h"
#include "bitarray/bitop.h"

namespace coder {
	using namespace mscds;
	/*
	GammaCoder
	
	template<typename OBitStream>
	static void encode_stream(OBitStream& os, const NumTp& n) {
		if (n == 0) throw std::runtime_error("cannot encode 0");
		else if (n == 1) { os.put(1, 1); os.endnum(); return ; }
		LenTp len = find_MSB_intr(n);
		os.fill0(len);
		os.put(n - (1ull << len), len);
		os.endnum();
	}

	template<typename IBitStream>
	static void decode_stream(IBitStream& is, NumTp& n, LenTp& len) {
		LenTp l = is.next1();
		//CodeTp v = is.take(l);
		n = is.take(l) | (1ull << l);
		len = l*2 + 1;
	}

	template<typename IBitStream>
	static void decode_stream_len(IBitStream& is, LenTp& len) {
		LenTp l = is.next1();
		is.skip(l);
		len = l*2 + 1;
	}*/

	// little endian style
	LenTp GammaCoder::encodelen(CodeTp n)	{
		if (n == 0) throw std::runtime_error("cannot encode 0");
		if (n == 1) return 1;
		else return 1 + 2*msb_intr(n);
	}

	CodePr GammaCoder::encode(NumTp n) {
		LenTp  bitlen;
		if (n == 0) throw std::runtime_error("cannot encode 0"); 
		int len;
		if (n == 1) { return CodePr(1,1); }
		else len = msb_intr(n);
		// send len*0, send 1, send n from low to high with len - 1 bits
		bitlen = len*2 + 1;
		if (bitlen > sizeof(NumTp)*8) throw std::runtime_error("code is too long, cannot contain in a word");
		return CodePr((((n - (1ull << len)) << 1) | 1) << len, bitlen);
	}

	coder::CodePr GammaCoder::encode_raw(NumTp n) {
		LenTp  bitlen;
		if (n == 0) throw std::runtime_error("cannot encode 0"); 
		int len;
		if (n == 1) { return CodePr(1,0); }
		else len = msb_intr(n);
		return CodePr(n - (1ull << len), len);
	}


	CodePr GammaCoder::decode2(CodeTp n) {
		unsigned int l = lsb_intr(n);
		return CodePr(((n >> (l + 1)) & ~(~0ull << l)) | (1ull << l), 2*l + 1);
	}

	// little endian style
	LenTp DeltaCoder::encodelen(CodeTp n) {
		if (n == 0) throw std::runtime_error("cannot encode 0"); 
		if (n == 1) return 1;
		LenTp l = msb_intr(n);
	    return GammaCoder::encodelen(l + 1) + l;
	}

	CodePr DeltaCoder::encode(NumTp n) {
	    if (n == 0) throw std::runtime_error("cannot encode 0"); 
	    if (n == 1) { return CodePr(1,1); }
		LenTp bitlen;
		LenTp l = msb_intr(n);
		std::pair<CodeTp, LenTp> rs = GammaCoder::encode(l + 1);
	    bitlen = rs.second + l;
		return CodePr((rs.first | ((n - (1ull << l)) << rs.second) ), bitlen);
	    //return concatc(GammaCoder::encode(l + 1), CodePr(n - (1ull << l), l));
	}


	CodePr DeltaCoder::decode2(CodeTp n) {
		std::pair<NumTp, LenTp>  rs = GammaCoder::decode2(n);
		LenTp l = rs.first - 1;
		LenTp lp = GammaCoder::encodelen(l + 1);
		return CodePr(((n >> lp) & ~(~0ull << l)) | (1ull << l), rs.second + l);
	}
}
