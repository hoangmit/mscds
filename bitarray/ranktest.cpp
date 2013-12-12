
#include "rank25p.h"
#include "rank6p.h"
#include "rank3p.h"
#include "rrr.h"
#include "rrr2.h"
#include "utils/utest.h"
#include "utils/utils.h"


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

template<typename RankSelect>
void test_rank(const std::vector<bool>& vec) {
	vector<int> ranks(vec.size() + 1);
	ranks[0] = 0;
	for (unsigned int i = 1; i <= vec.size(); i++)
		if (vec[i-1]) ranks[i] = ranks[i-1] + 1;
		else ranks[i] = ranks[i-1];
	BitArray v;
	v = BitArrayBuilder::create(vec.size());
	//v.fillzero();
	for (unsigned int i = 0; i < vec.size(); i++) {
		v.setbit(i, vec[i]);
	}

	for (unsigned int i = 0; i < vec.size(); i++) {
		ASSERT(vec[i] == v.bit(i));
	}

	RankSelect r;
	//Rank6pBuilder bd;
	RankSelect::BuilderTp::build(v, &r);
	for (unsigned int i = 0; i < vec.size(); ++i)
		ASSERT_EQ(vec[i], r.access(i));
	for (int i = 0; i <= vec.size(); ++i) {
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT_EQ(ranks[i], r.rank(i));
		}
	}
	unsigned int onecnt = 0;
	for (unsigned int i = 0; i < vec.size(); ++i)
		if (vec[i]) onecnt++;
	int last = -1;
	for (unsigned int i = 0; i < onecnt; ++i) {
		int pos = r.select(i);
		ASSERT_EQ(i, r.rank(pos));
		ASSERT_EQ(i + 1, r.rank(pos + 1));
		if (pos >= vec.size() || !vec[pos] || pos <= last) {
			cout << "select " << i << "  " << r.select(i) << endl;
			if (i > 0) r.select(i-1);
			ASSERT_EQ(true, vec[pos]);
		}
		ASSERT(pos > last);
		last = pos;
	}
	last = -1;
	for (unsigned int i = 0; i < vec.size() - onecnt; ++i) {
		int pos = r.selectzero(i);
		ASSERT_EQ(i, r.rankzero(pos)) << "pos =" << pos << "   i =" << i << endl;
		ASSERT_EQ(i + 1, r.rankzero(pos + 1));
		ASSERT(pos < vec.size() && vec[pos] == false);
		ASSERT(pos > last);
		last = pos;
	}
}

void test_temp(int len) {
	const std::vector<bool>& vec = bits_imbal(len);
	BitArray v;
	v = BitArrayBuilder::create(vec.size());
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
}

TEST(ranktest, rank25p) {
	test_rank<Rank25p>(bits_one());
	test_rank<Rank25p>(bits_zero());
	test_rank<Rank25p>(bits_onezero());
	test_rank<Rank25p>(bits_oneonezero());
	test_rank<Rank25p>(bits_zerozeroone());
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("Random");
		test_rank<Rank25p>(bits_dense(2046 + rand() % 4));
		test_rank<Rank25p>(bits_sparse(2046 + rand() % 4));
		test_rank<Rank25p>(bits_imbal(2046 + rand() % 4));
		if (i % 10 == 0) cout << ".";
	}
	test_rank<Rank25p>(bits_dense(100000));
	test_rank<Rank25p>(bits_sparse(100000));
	cout << endl;
}

TEST(ranktest, rank6p) {
	for (int i = 0; i < 100; i++) {
		test_temp(4094 + rand() % 4);
		if (i % 10 == 0) cout << ".";
	}

	test_rank<Rank6p>(bits_one());
	test_rank<Rank6p>(bits_zero());
	test_rank<Rank6p>(bits_onezero());
	test_rank<Rank6p>(bits_oneonezero());
	test_rank<Rank6p>(bits_zerozeroone());
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("Random");
		test_rank<Rank6p>(bits_dense(2046 + rand() % 4));
		test_rank<Rank6p>(bits_sparse(2046 + rand() % 4));
		test_rank<Rank6p>(bits_imbal(2046 + rand() % 4));
		if (i % 10 == 0) cout << ".";
	}
	test_rank<Rank6p>(bits_dense(100000));
	test_rank<Rank6p>(bits_sparse(100000));
	cout << endl;
}

TEST(ranktest, rank3p) {
	test_rank<Rank3p>(bits_one());
	test_rank<Rank3p>(bits_zero());
	test_rank<Rank3p>(bits_onezero());
	test_rank<Rank3p>(bits_oneonezero());
	test_rank<Rank3p>(bits_zerozeroone());
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("Random");
		test_rank<Rank3p>(bits_dense(2046 + rand() % 4));
		test_rank<Rank3p>(bits_sparse(2046 + rand() % 4));
		test_rank<Rank3p>(bits_imbal(2046 + rand() % 4));
		if (i % 10 == 0) cout << ".";
	}
	test_rank<Rank3p>(bits_dense(100000));
	test_rank<Rank3p>(bits_sparse(100000));
	cout << endl;
}

TEST(ranktest, rrr) {
	test_rank<RRR>(bits_one());
	{
		SCOPED_TRACE("Normal");
		cout << "afsdf" << endl;
		test_rank<RRR>(bits_zero());
		cout << "fsfsdfs" << endl;
	}
	test_rank<RRR>(bits_onezero());

	test_rank<RRR>(bits_oneonezero());
	test_rank<RRR>(bits_zerozeroone());
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("Random");
		test_rank<RRR>(bits_dense(2046 + rand() % 4));
		test_rank<RRR>(bits_sparse(2046 + rand() % 4));
		test_rank<RRR>(bits_imbal(2046 + rand() % 4));
		if (i % 10 == 0) cout << ".";
	}
	test_rank<RRR>(bits_dense(20000));
	test_rank<RRR>(bits_sparse(20000));
	cout << endl;
}

TEST(ranktest, rrr2) {
	test_rank<RRR2>(bits_one());
	test_rank<RRR2>(bits_zero());
	test_rank<RRR2>(bits_onezero());
	test_rank<RRR2>(bits_oneonezero());
	test_rank<RRR2>(bits_zerozeroone());
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("Random");
		test_rank<RRR2>(bits_dense(2046 + rand() % 4));
		test_rank<RRR2>(bits_sparse(2046 + rand() % 4));
		test_rank<RRR2>(bits_imbal(2046 + rand() % 4));
		if (i % 10 == 0) cout << ".";
	}
	test_rank<RRR2>(bits_dense(20000));
	test_rank<RRR2>(bits_sparse(20000));
	cout << endl;
}


