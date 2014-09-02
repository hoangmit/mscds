#pragma once

/** 
\file

Gamma and Delta coding
  http://en.wikipedia.org/wiki/Elias_delta_coding


Implemented by Hoang
*/

#include <cassert>
#include "coder.h"

namespace coder {

	/// Gamma codec
	class GammaCoder: public Coder {
	public:
		// little endian style
		static LenTp encodelen(CodeTp n);
		static CodePr encode(NumTp n);
		static CodePr decode2(CodeTp n);

		static CodePr encode_raw(NumTp n);
	};

	/// Delta codec
	class DeltaCoder: public Coder {
	public:
		// little endian style
		static LenTp encodelen(CodeTp n);
		static CodePr encode(NumTp n);
		static CodePr decode2(CodeTp n);
	};

	/// map signed integer to unsigned integer space
	inline uint64_t absmap(int64_t v) {
		if (v == 0) return 0;
		else if (v > 0) return v*2 - 1;
		else return (-v)*2;
	}

	inline int64_t absunmap(uint64_t v) {
		if (v == 0) return 0;
		else if (v % 2 == 0) return -((int64_t)(v/2));
		else return (v + 2)/2;
	}
}
