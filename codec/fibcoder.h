#pragma once

/** 
\file

Fibonacci coding 
  http://en.wikipedia.org/wiki/Fibonacci_coding  

Implemented by Hoang
*/

#include "coder.h"

namespace coder {

	/// Fibinacci codec
	class FibCoder: public Coder {
	public:
		static LenTp encodelen(CodeTp n);
		static CodePr encode(NumTp n);
		static CodePr decode2(CodeTp n);
		//static NumTp decode(CodeTp n);
	};


	/*
	uint8_t fib_wcount(uint64_t n);
	uint8_t fib_sel(uint64_t n, uint8_t p);
	*/

	

}
