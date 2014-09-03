
#include "sdarray_th.h"
#include "sdarray_zero.h"
#include "utils/utest.h"

namespace tests {

using namespace std;
using namespace mscds;

void test_cmp(const std::vector<unsigned int>& vals) {
	size_t len = vals.size();
	SDArrayTHBuilder bd;
	SDArrayTH arr;
	SDArrayZero zero;
	for (unsigned int i = 0; i < len; ++i) {
		zero.add(vals[i]);
		bd.add(vals[i]);
	}

	bd.build(&arr);

	for (unsigned int i = 0; i < len; ++i) {
		ASSERT_EQ(vals[i], zero.lookup(i));
		auto w = arr.lookup(i);
		if (w != vals[i]) {
			w = arr.lookup(i);
		}
		ASSERT_EQ(vals[i], arr.lookup(i));

		ASSERT_EQ(zero.prefixsum(i), arr.prefixsum(i));
		uint64_t ps;
		arr.lookup(i, ps);
		ASSERT_EQ(zero.prefixsum(i), ps);
	}
	ASSERT_EQ(zero.prefixsum(len), arr.prefixsum(len));
	auto last = zero.prefixsum(len);
	for (unsigned int p = 0; p < last; ++p) {
		auto v1 = zero.rank(p);
		auto v2 = arr.rank(p);
		ASSERT_EQ(v1, v2);
	}
}

TEST(sdarray_th, test1) {
	for (unsigned int k = 0; k < 1000; ++k) {
		size_t len = 100;
		std::vector<unsigned int> vals;
		for (unsigned int i = 0; i < len; ++i)
			vals.push_back(rand() % 20);
		test_cmp(vals);
	}
}

TEST(sdarray_th, test2) {
	for (unsigned int k = 0; k < 50; ++k) {
		size_t len = 2000;
		std::vector<unsigned int> vals;
		for (unsigned int i = 0; i < len; ++i)
			vals.push_back(rand() % 20);
		test_cmp(vals);
	}
}

}//namespace

/*
int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "*";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::GTEST_FLAG(catch_exceptions) = "0";

	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}*/
