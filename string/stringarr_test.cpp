#include "utils/utest.h"
#include "utils/utils.h"

#include "mem/save_load_test.h"

#include "stringarr.h"
#include "blob_array.h"

#include <vector>
#include <iostream>

namespace tests {

using namespace std;
using namespace mscds;

template<typename SA>
void check_starr(int n, const char* A[], const SA& sa) {
	ASSERT_EQ(n, sa.length());
	for (int i = 0; i < n; ++i) {
		std::string p = sa.get_str(i);
		ASSERT_EQ(p, A[i]);
	}
	for (int i = 0; i < n; ++i) {
		StringPtr pt = sa.get(i);
		unsigned slen = strlen(A[i]);
		ASSERT_EQ(slen, pt->length());
		ASSERT_EQ(0, strncmp(A[i], pt->c_str(), slen));
	}
}

TEST(string_array, strarr1) {
	const int n = 10;
	const char* A[n] = {"", "", "abc", "defx", "", "eg", "", "", "xagtg", ""};
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;
	bd.build(&sa);
	check_starr(n, A, sa);
}

TEST(string_array, strarr2) {
	const int n = 8;
	const char* A[n] = {"abc", "", "defx", "eg", "", "", "", "xagtg"};
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;
	bd.build(&sa);
	check_starr(n, A, sa);
}

TEST(string_array, saveload) {
	const int n = 8;
	const char* A[n] = {"abc", "", "defx", "eg", "", "", "", "xagtg"};
	StringArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	StringArr sa;

	string fn = save_load_test(bd, sa);

	check_starr(n, A, sa);

	sa.clear();
	std::remove(fn.c_str());
}

TEST(string_array, blob_saveload) {
	const int n = 8;
	const char* A[n] = {"abc", "", "defx", "eg", "", "", "", "xagtg"};
	BlobArrBuilder bd;
	for (int i = 0; i < n; ++i)
		bd.add(A[i]);
	BlobArr sa;

	string fn = save_load_test(bd, sa);

	check_starr(n, A, sa);

	sa.clear();
	std::remove(fn.c_str());
}


}//namespace
