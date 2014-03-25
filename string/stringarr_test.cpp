#include "utils/utest.h"
#include "utils/utils.h"

#include "mem/save_load_test.h"

#include "stringarr.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace mscds;



TEST(string_array, strarr1) {
	const int n = 10;
	const char* A[n] = { "", "", "abc", "defx", "", "eg", "", "", "xagtg", "" };
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;
	bd.build(&sa);
	for (int i = 0; i < n; ++i) {
		const char * p = sa.get(i);
		ASSERT_EQ(0, strcmp(A[i], p));
	}
}

TEST(string_array, strarr2) {
	const int n = 8;
	const char* A[n] = { "abc", "", "defx", "eg", "", "", "", "xagtg" };
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;
	bd.build(&sa);
	for (int i = 0; i < n; ++i) {
		const char * p = sa.get(i);
		ASSERT_EQ(0, strcmp(A[i], p));
	}
}


TEST(string_array, saveload) {
	const int n = 8;
	const char* A[n] = { "abc", "", "defx", "eg", "", "", "", "xagtg" };
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;
	
	string fn = save_load_test(bd, sa);

	for (int i = 0; i < n; ++i) {
		const char * p = sa.get(i);
		ASSERT_EQ(0, strcmp(A[i], p));
	}


	std::remove(fn.c_str());
}