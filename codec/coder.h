
#pragma once

/** 

Interface of Coder function and 2 common functions.

*/

#include <stdint.h>
#include <utility>
#include <stdexcept>
#include <vector>
#include <cassert>

namespace coder {
	typedef uint64_t NumTp;
	typedef uint64_t CodeTp;
	typedef uint16_t LenTp;

	typedef std::pair<CodeTp, LenTp> CodePr;

	class Coder {
	public:
		LenTp encodelen(CodeTp n);
		std::pair<CodeTp, LenTp> encode(NumTp n);
		std::pair<NumTp, LenTp> decode2(CodeTp n);
		//static NumTp decode(CodeTp n);
	};

	inline CodePr concatc(const CodePr& code1, const CodePr& code2) {
		assert(code1.second + code2.second <= sizeof(CodeTp)*8);
		return CodePr(code1.first | (code2.first << code1.second), code1.second + code2.second);
	}

	inline CodePr concatc(const CodePr& code1, const CodePr& code2, const CodePr& code3) {
		assert(code1.second + code2.second + code3.second <= sizeof(CodeTp)*8);
		return CodePr(code1.first | (code2.first << code1.second) | (code3.first << (code1.first + code2.first)), code1.second + code2.second);
	}
}
