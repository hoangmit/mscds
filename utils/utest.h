#pragma once
#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_

#ifndef ASSERT

#include <cstdio>

// http://stackoverflow.com/questions/5252375/custom-c-assert-macro
// http://stackoverflow.com/questions/37473/how-can-i-assert-without-using-abort
// http://en.wikipedia.org/wiki/Assert.h

#define __FAILED_MSG(err, file, line, func) \
	((void)printf("Assertion failed: `%s`, file: %s, func: %s, line: %u\n", err, file, func, line), abort(), 0)

#define ASSERT(cond) \
	(void)( (!!(cond)) || __FAILED_MSG(#cond, __FILE__, __LINE__, __FUNCTION__))

#define ASSERT_EQ(exp,val) ASSERT((exp)==(val))

#endif

#endif //__UNIT_TEST_H_