#include "RMQ_table.h"

#include <iostream>
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
			assert(exp == val);
		}
	}
	bd.build(vec, &t, false, false);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::max_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			assert(exp == val);
		}
	}
	bd.build(vec, &t, true, true);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::min_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			assert(exp == val);
		}
	}
	bd.build(vec, &t, false, true);
	for (int l = 1; l < vec.size(); l++) {
		for (int i = 0; i < vec.size() - l + 1; ++i) {
			int exp = std::max_element(vec.begin() + i, vec.begin() + i + l) - vec.begin();
			int val = t.m_idx(i, i+l);
			assert(exp == val);
		}
	}
	cout << '.';
}

void test1() {
	const int len = 9;
	int A[len] = {2,4,7,8,1,6,3,9,5};
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


vector<uint64_t>  rand_vec(int len, int range = 1000000) {
	vector<uint64_t> out;
	for (unsigned int i = 0; i < len; ++i) {
		out.push_back(rand() % range);
	}
	return out;
}

void test2() {
	for (int i = 0; i < 100; i++) {
		test_rmq(rand_vec(126 + rand() % 4));
	}
}

int main() {
	test1();
	test2();
	return 0;
}
