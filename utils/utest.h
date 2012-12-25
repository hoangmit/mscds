#pragma once
#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_

#define PRT(x) cout<< #x <<"="<<(x) << endl


#ifndef ASSERT

#include <iostream>

// http://stackoverflow.com/questions/5252375/custom-c-assert-macro
// http://stackoverflow.com/questions/37473/how-can-i-assert-without-using-abort
// http://en.wikipedia.org/wiki/Assert.h

#define __EXPVAL(exp,val) \
	((void)(std::cerr << "Expected: " << #exp << " = " << (exp) << endl << "Value: " << #val << " = " << (val) << endl))

#define __FAILED_MSG(err, file, line, func) \
	((void)(std::cerr << "Assertion failed: `" << err << "`, file: "<<file<<", func: "<< func <<", line: "<< line << "\n"), abort(), 0)

#define ASSERT(cond) \
	(void)( (!!(cond)) || __FAILED_MSG(#cond, __FILE__, __LINE__, __FUNCTION__))

#define ASSERT_EQ(exp,val) ASSERT((exp)==(val))

#endif


#include <stdint.h>
#include <cstdlib>

namespace utils{

#if defined(_MSC_VER)

/* MS VC's RAND_MAX is only 32767 */
inline uint32_t rand32() {
	return (((((uint32_t)rand()) << 15) ^ ((uint32_t)rand())) << 2) | (rand() & 3);
}
#else
inline uint32_t rand32() {
	return (uint32_t) mrand48();
}
#endif

inline uint64_t rand64() {
	return 
	  (((uint64_t) rand32() <<  0) & 0x00000000FFFFFFFFull) | 
	  (((uint64_t) rand32() << 32) & 0xFFFFFFFF00000000ull);
}

// fast random generator from
// http://www.jstatsoft.org/v08/i14/paper
// http://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c/
// http://www.codeproject.com/Articles/9187/A-fast-equivalent-for-System-Random
// by George Marsaglia
struct XorShiftRng {
	XorShiftRng() {
		init();
	}

	void init() {
		init96();
	}

	uint32_t operator()(void) {
		return xorshf96();
	}

	void init96() {
		x=123456789;
		y=362436069; z=521288629;
	}
	uint32_t xorshf96() { //period 2^96-1
		unsigned long t;
		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;
		t = x; x = y; y = z;
		z = t ^ x ^ y;
		return z;
	}

	uint32_t x, y, z, w;

	void srand(uint32_t seed) {
		init();
		x = (uint32_t)((seed * 1431655781) 
			+ (seed * 1183186591)
			+ (seed * 622729787)
			+ (seed * 338294347));
		if (x == 0) x = 1;
	}


	void init128_1() {
		x = 123456789; 
		y = 362436069; z = 521288629; w = 88675123;
	}
	void init128_2() {
		x = 123456789;
		y = 842502087, z=3579807591, w=273326509;
	}
	// for other options of [a, b, c] see the paper
	uint32_t xorshf128() {
		uint32_t t = x ^ (x << 11);
		x = y; y = z; z = w;
		return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
	}

};

}//namespace
#endif //__UNIT_TEST_H_
