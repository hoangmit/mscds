#include "RMQ_table.h"
#include "RMQ_sct.h"
#include "utils/utest.h"
#include <iostream>


namespace tests {
using namespace std;
using namespace mscds;


void test_rmq(const vector<uint64_t>& vec) {
	RMQ_table t;
	RMQ_table_builder bd;
	bd.build(vec, &t, true, false);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::min_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			ASSERT(i <= val && val < i + l);
			ASSERT(exp == val || vec[exp] == vec[val]);
		}
	}
	bd.build(vec, &t, false, false);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::max_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			ASSERT(i <= val && val < i + l);
			ASSERT(exp == val || vec[exp] == vec[val]);
		}
	}
	bd.build(vec, &t, true, true);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::min_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			ASSERT(i <= val && val < i + l);
			ASSERT(exp == val || vec[exp] == vec[val]);
		}
	}
	bd.build(vec, &t, false, true);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::max_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			ASSERT(i <= val && val < i + l);
			ASSERT(exp == val || vec[exp] == vec[val]);
		}
	}
	cout << '.';
}

TEST(test1, RMQ) {
	const int len = 9;
	int A[len] = {2, 4, 7, 8, 1, 6, 3, 9, 5};
	vector<uint64_t> vec;
	for (int i = 0; i < len; i++) vec.push_back(A[i]);

	test_rmq(vec);
	/*min_structure
	{2,4,7,8,1,6,3,9,5}
	-
	{0,1,2,3,4,5,6,7,8}
	{0,1,2,4,4,6,6,8}
	{0,4,4,4,4,6,6}
	{4,4,4,4,4}*/
}

void test_sct(const vector<uint64_t>& vec) {
	RMQ_sct sct;
	sct.build(vec);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::min_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = sct.m_idx(i, i+l);
			ASSERT(i <= val && val < i + l);
			ASSERT(exp == val || vec[exp] == vec[val]);
		}
	}

}


TEST(test2, RMQ) {
	for (int i = 0; i < 100; i++) {
		test_rmq(rand_vec<uint64_t>(126 + rand() % 4));
		test_rmq(rand_vec<uint64_t>(126 + rand() % 4, 100));
	}
}

void test_cmp(const vector<uint64_t>& x) {
	//const int len = 9;
	//int A[len] = {2,4,7,8,1,6,3,9,5};
	//vector<uint64_t> x;
	//for (int i = 0; i < len; i++) x.push_back(A[i]); 

	//auto x = rand_vec(rand() % 10 + 10000);
	RMQ_sct sct;
	sct.build(x);
	RMQ_table t;
	RMQ_table_builder bd;
	bd.build(x, &t, true, true);
	for (size_t i = 0; i < 1000; i++) {
		size_t l = rand() % (x.size() - 1);
		size_t r = rand() % (x.size());
		//size_t l = 0;
		//size_t r = 3;
		if (l == r) r++;
		if (l > r) std::swap(l, r);
		uint64_t i1 = t.m_idx(l, r);
		uint64_t i2 = t.m_idx(l, r);
		ASSERT(i1 == i2 || x[i1] == x[i2]);
	}
}

TEST(random_long, RMQ) {
	for (size_t i = 0; i < 100; i++) {
		if (i % 10 == 0) cout << '.';
		test_sct(rand_vec<uint64_t>(rand() % 10 + 100, 20));
		test_sct(rand_vec<uint64_t>(rand() % 10 + 100));
	}
	for (size_t i = 0; i < 1000; i++) {
		if (i % 10 == 0) cout << '.';
		test_cmp(rand_vec<uint64_t>(rand() % 10 + 100, 20));
		test_cmp(rand_vec<uint64_t>(rand() % 10 + 10000));
	}

}

}//namespace

/*
int main() {
	test1();
	test2();
	return 0;
}
*/