#pragma once
#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_

#define PRT(x) cout<< #x <<"="<<(x) << endl

#include <iostream>

#if defined(_USE_OWN_TEST_LIB_)

#ifndef ASSERT

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

#endif //ASSERT

#define TEST(case, test) void test ## case () 

#else //USE_OWN_TEST_LIB

#include <gtest/gtest.h>
#define ASSERT ASSERT_TRUE

#endif //USE_OWN_TEST_LIB


#endif //__UNIT_TEST_H_
