
#include "SampledSum.h"
#include "rank_vals.h"
#include "poly_vals.h"

#include "utils/utest.h"

#include <iostream>
#include <deque>

using namespace std;
using namespace app_ds;

std::deque<unsigned int> generate1(unsigned int size) {
	deque<unsigned int> vals;
	for (unsigned int i = 0; i < size; i++) {
		unsigned int v = 1 + rand() % 10000;
		vals.push_back(v);
	}
	return vals;
}

template<typename ValArrTp>
void test_rd1(const std::deque<unsigned int>& vals, unsigned int method, unsigned int rate = 64) {
	typename ValArrTp::BuilderTp bd;
	bd.init(method, rate);
	for (unsigned int i = 0; i < vals.size(); i++) {
		bd.add(vals[i]);
	}
	ValArrTp qs;
	bd.build(&qs);
	for (unsigned int i = 0; i < vals.size(); i++) {
		ASSERT_EQ(vals[i], qs.access(i));
		if (i < vals.size() - 3) {
			typename ValArrTp::Enum en;
			qs.getEnum(i, &en);
			for (unsigned j = 0; j < 3; ++j)
				ASSERT_EQ(vals[i + j], en.next());
		}
	}
}

void test1() {
	for (int i = 0; i < 10; ++i) {
		test_rd1<RankValArr>(generate1(1000), 1);
		test_rd1<RankValArr>(generate1(1000), 2);
		test_rd1<RankValArr>(generate1(1000), 3);
	}
	for (int i = 0; i < 10; ++i) {
		test_rd1<PRValArr>(generate1(1000), 1);
		test_rd1<PRValArr>(generate1(1000), 2);
		test_rd1<PRValArr>(generate1(1000), 3);
	}}

int main() {
	
	test1();
	return 0;
}
