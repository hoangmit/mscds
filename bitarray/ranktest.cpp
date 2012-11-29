
#include "rank25p.h"
#include "rank6p.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace mscds;

void test_rank(const std::vector<bool>& vec) {
	vector<int> ranks(vec.size() + 1);
	ranks[0] = 0;
	for (unsigned int i = 1; i <= vec.size(); i++)
		if (vec[i-1]) ranks[i] = ranks[i-1] + 1;
		else ranks[i] = ranks[i-1];
	BitArray v;
	v = BitArray::create(vec.size());
	v.fillzero();
	for (unsigned int i = 0; i < vec.size(); i++) {
		v.setbit(i, vec[i]);
	}

	for (unsigned int i = 0; i < vec.size(); i++) {
		assert(vec[i] == v.bit(i));
	}

	Rank6p r;
	Rank6pBuilder bd;
	bd.build(v, &r);
	for (int i = 0; i <= vec.size(); ++i) {
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			assert(ranks[i] == r.rank(i));
		}
	}
	unsigned int onecnt = 0;
	for (unsigned int i = 0; i < vec.size(); ++i)
		if (vec[i]) onecnt++;
	int last = -1;
	for (unsigned int i = 0; i < onecnt; ++i) {
		int pos = r.select(i);
		if (pos >= vec.size() || !vec[pos] || pos <= last) {
			cout << "select " << i << "  " << r.select(i) << endl;
			if (i > 0) r.select(i-1);
			assert(vec[pos] == true);
		}
		assert(pos > last);
		last = pos;
	}
	last = -1;
	for (unsigned int i = 0; i < vec.size() - onecnt; ++i) {
		int pos = r.selectzero(i);
		assert(pos < vec.size() && vec[pos] == false);
		assert(pos > last);
		last = pos;
	}
	cout << ".";
}

void test_one() {
	std::vector<bool> v;
	for (int i = 0; i < 100000; ++i)
		v.push_back(true);
	test_rank(v);
}

void test_zero() {
	std::vector<bool> v;
	for (int i = 0; i < 100000; ++i)
		v.push_back(false);
	test_rank(v); 
}

void test_onezero() {
	std::vector<bool> v;
	for (int i = 0; i < 100000; ++i) {
		v.push_back(true);
		v.push_back(false);
	}
	test_rank(v);
}

void test_oneonezero() {
	std::vector<bool> v;
	for (int i = 0; i < 100000; ++i) {
		v.push_back(true);
		v.push_back(true);
		v.push_back(false);
	}
	test_rank(v);
}

void test_zerozeroone() {
	std::vector<bool> v;
	for (int i = 0; i < 100000; ++i) {
		v.push_back(false);
		v.push_back(false);
		v.push_back(true);
	}
	test_rank(v);
}


void test_rnd1(int len) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		if (rand() % 2 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	test_rank(v);
}

void test_rnd2(int len) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		if (rand() % 100 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	test_rank(v);
}

void test_all_rank() {
	test_one();
	test_zero();
	test_onezero();
	test_oneonezero();
	test_zerozeroone();
	for (int i = 0; i < 100; i++) {
		test_rnd1(1000);
		test_rnd2(1000);
	}
	test_rnd1(100000);
	test_rnd2(100000);
	cout << endl;
}

//void gridquerytest2();
//void test_map_all();
//void wat_test_all();

int main() {
	//test_map_all();
	test_all_rank();
	//wat_test_all();
	return 0;
}