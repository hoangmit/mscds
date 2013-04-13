
#include "rank25p.h"
#include "rank6p.h"
#include "utils/utest.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace mscds;


std::vector<bool> bits_one(int len = 50000) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i)
		v.push_back(true);
	return v;
}

std::vector<bool> bits_zero(int len = 50000) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i)
		v.push_back(false);
	return v;
}

std::vector<bool> bits_onezero(int len = 50000) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		v.push_back(true);
		v.push_back(false);
	}
	return v;
}

std::vector<bool> bits_oneonezero(int len = 50000) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		v.push_back(true);
		v.push_back(true);
		v.push_back(false);
	}
	return v;
}

std::vector<bool> bits_zerozeroone(int len = 50000) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		v.push_back(false);
		v.push_back(false);
		v.push_back(true);
	}
	return v;
}

std::vector<bool> bits_dense(int len) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		if (rand() % 2 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	return v;
}

std::vector<bool> bits_sparse(int len) {
	std::vector<bool> v;
	for (int i = 0; i < len; ++i) {
		if (rand() % 100 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	return v;
}

std::vector<bool> bits_imbal(int len) {
	std::vector<bool> v;
	for (int i = 0; i < len/2; ++i) {
		if (rand() % 100 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	for (int i = 0; i < len/2; ++i) {
		if (rand() % 2 == 1)
			v.push_back(true);
		else v.push_back(false);
	}
	return v;
}

//--------------------------------------------------------------------------

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
		ASSERT(vec[i] == v.bit(i));
	}

	Rank6p r;
	//Rank6pBuilder bd;
	Rank6pBuilder::build(v, &r);
	for (int i = 0; i <= vec.size(); ++i) {
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT(ranks[i] == r.rank(i));
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
			ASSERT(vec[pos] == true);
		}
		ASSERT(pos > last);
		last = pos;
	}
	last = -1;
	for (unsigned int i = 0; i < vec.size() - onecnt; ++i) {
		int pos = r.selectzero(i);
		ASSERT(pos < vec.size() && vec[pos] == false);
		ASSERT(pos > last);
		last = pos;
	}
	cout << ".";
}

void test_temp(int len) {
	const std::vector<bool>& vec = bits_imbal(len);
	BitArray v;
	v = BitArray::create(vec.size());
	v.fillzero();
	for (unsigned int i = 0; i < vec.size(); i++) {
		v.setbit(i, vec[i]);
	}
	Rank6p t;
	//Rank6pBuilder bd;
	Rank6pBuilder::build(v, &t);
	Rank6pHintSel rhs;
	rhs.init(v);

	unsigned int onecnt = 0;
	for (unsigned int i = 0; i < vec.size(); ++i)
		if (vec[i]) onecnt++;
	int last = -1;
	for (unsigned int i = 0; i < onecnt; ++i) {
		int pos = rhs.select(i);
		//int pos2 = t.select(i);
		if (pos >= vec.size() || !vec[pos] || pos <= last) {
			cout << "select " << i << "  " << rhs.select(i) << endl;
			if (i > 0) rhs.select(i-1);
			ASSERT(vec[pos] == true);
		}
		ASSERT(pos > last);
		last = pos;
	}
	cout << '.';
}

TEST(all_rank, ranktest) {
	for (int i = 0; i < 100; i++) {
		test_temp(4094 + rand() % 4);
	}
	return ;
	test_rank(bits_one());
	test_rank(bits_zero());
	test_rank(bits_onezero());
	test_rank(bits_oneonezero());
	test_rank(bits_zerozeroone());
	for (int i = 0; i < 100; i++) {
		test_rank(bits_dense(2046 + rand() % 4));
		test_rank(bits_sparse(2046 + rand() % 4));
		test_rank(bits_sparse(2046 + rand() % 4));
	}
	test_rank(bits_dense(100000));
	test_rank(bits_sparse(100000));
	cout << endl;
}

//void gridquerytest2();
//void test_map_all();
//void wat_test_all();
/*
int main() {
	//test_map_all();
	test_all_rank();
	//wat_test_all();
	return 0;
}*/