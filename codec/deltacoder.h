#pragma once

#include <cassert>
#include "coder.h"

namespace coder {

	class GammaCoder: public Coder {
	public:
		// little endian style
		static LenTp encodelen(CodeTp n);
		static CodePr encode(NumTp n);
		static CodePr decode2(CodeTp n);
	};

	class DeltaCoder: public Coder {
	public:
		// little endian style
		static LenTp encodelen(CodeTp n);
		static CodePr encode(NumTp n);
		static CodePr decode2(CodeTp n);
	};

}
