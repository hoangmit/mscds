#pragma once
#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_

#define PRT(x) cout<< #x <<"="<<(x) << endl


#ifndef ASSERT

#include <iostream>
#include <ctime>
#include <type_traits>

// http://stackoverflow.com/questions/5252375/custom-c-assert-macro
// http://stackoverflow.com/questions/37473/how-can-i-assert-without-using-abort
// http://en.wikipedia.org/wiki/Assert.h




namespace has_insertion_operator_impl {
	typedef char no;
	typedef char yes[2];

	struct any_t { template<typename T> any_t( T const& ); };

	no operator<<( std::ostream const&, any_t const& );

	yes& test( std::ostream& );
	no test( no );

	template<typename T>
	struct has_insertion_operator {
		static std::ostream &s;
		static T const &t;
		static bool const value = sizeof( test(s << t) ) == sizeof( yes );
	};

	template<typename T> inline
		typename std::enable_if<has_insertion_operator<T>::value,
		void>::type
		_print_cerr(const T &t) {
			std::cerr << t << "\n";
	}
	template<typename T> inline
		typename std::enable_if<!has_insertion_operator<T>::value,
		void>::type
		_print_cerr(const T &t) {}

	template<typename T1, typename T2>
	inline void _assert_cmp(const T1& exp, const T2& val, const char* expstr, const char* valstr, const char* file, int line, const char* func) {
		if (exp != val) {
			std::cerr << "Assertion failed: " << expstr << " == " << valstr << ", file: "<<file<<", func: "<< func <<", line: "<< line << "\n";
			std::cerr << "Expected value = ";
			_print_cerr<T1>(exp);
			std::cerr << "Actual value   = ";
			_print_cerr<T2>(val);
			abort();
		}
	}
}


#define __FAILED_MSG(err, file, line, func) \
	((void)(std::cerr << "Assertion failed: `" << err << "`, file: "<<file<<", func: "<< func <<", line: "<< line << "\n"), abort(), 0)

#define ASSERT(cond) \
	(void)( (!!(cond)) || __FAILED_MSG(#cond, __FILE__, __LINE__, __FUNCTION__))

//#define ASSERT_EQ(exp,val) ASSERT((exp)==(val))
#define ASSERT_EQ(exp,val) has_insertion_operator_impl::_assert_cmp(exp, val, #exp, #val, __FILE__, __LINE__, __FUNCTION__)

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


struct Timer {
	std::clock_t last, start;
	Timer() { reset(); }

	void reset() {
		start = std::clock();
		last = start;
	}
	double current() {
		clock_t c = std::clock();
		double t = (double)(c - last)  / CLOCKS_PER_SEC;
		last = c;
		return t;
	}

	double total() {
		return (double)(std::clock() - start) / CLOCKS_PER_SEC;
	}
};


}//namespace
#endif //__UNIT_TEST_H_
