#include "nintv.h"

#include "utils/utest.h"

#include <iostream>
#include <vector>
#include <deque>

#include "utils/benchmark.h"

#include "utils/utils.h"
#include "intv_rand_gen.hpp"

using namespace std;
using namespace app_ds;

std::vector<std::pair<unsigned int, unsigned int> > convert2pair(const std::deque<ValRange>& vec){
	std::vector<std::pair<unsigned int, unsigned int> > ret;
	ret.reserve(vec.size());
	for (auto& v : vec) {
		ret.emplace_back(v.st, v.ed);
	}
	return ret;
}

template<typename StructTp>
void test_intervals(const std::vector<std::pair<unsigned int, unsigned int> >& rng, int testid = 0) {
	typename StructTp::BuilderTp bd;
	for (size_t i = 0; i < rng.size(); ++i)
		bd.add(rng[i].first, rng[i].second);
	StructTp r;
	bd.build(&r);
	ASSERT_EQ(rng.size(), r.length());
	for (size_t i = 0; i < rng.size(); ++i) {
		ASSERT_EQ(rng[i].first, r.int_start(i));
		ASSERT_EQ(rng[i].second, r.int_end(i));
		ASSERT_EQ(rng[i].second - rng[i].first, r.int_len(i));
		auto px = r.int_startend(i);
		ASSERT(rng[i].first == px.first && rng[i].second == px.second);
	}
	for (size_t i = 0; i < rng.size() - 3; ++i) {
		typename StructTp::Enum e;
		r.getEnum(i, &e);
		for (size_t j = 0; j < 3; ++j) {
			auto p = e.next();
			ASSERT_EQ(rng[i+j].first, p.first);
			ASSERT_EQ(rng[i+j].second, p.second);
		}
	}
	size_t mlen = rng.back().second;
	size_t j = 0, jp = r.npos();
	size_t cnt = 0;
	for (size_t i = 0; i < mlen; ++i) {
		if (rng[j].second <= i) ++j;
		if (rng[j].first <= i) jp = j;
		auto v = r.rank_interval(i);
		ASSERT_EQ(jp, v);
		ASSERT_EQ(cnt, r.coverage(i)) << "test id = " << testid << "   i = " << i << endl;
		auto p = r.find_cover(i);
		if (rng[j].first <= i) {
			++cnt;
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(i - rng[j].first + 1, p.second);
		}else {
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(0, p.second);
		}
	}
	ASSERT_EQ(rng.size() - 1, j);
	auto v = r.rank_interval(mlen);
	ASSERT_EQ(rng.size() - 1, v);
}

TEST(intv, basic1) {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	test_intervals<NIntv>(convert2pair(genInp(Av)));
}

TEST(intv, basic2) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = gen_density(len);
		test_intervals<NIntv>(convert2pair(genInp(A)));
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}

TEST(intv, group1) {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	auto iv = convert2pair(genInp(Av));
	test_intervals<NIntvGroup>(iv);
}

TEST(intv, group2) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = gen_density(len);
		auto iv = convert2pair(genInp(A));
		test_intervals<NIntvGroup>(iv, i);
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}

TEST(intv, gap1) {
	const int len = 11;
	int A[len] = { 1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3 };
	vector<int> Av(A, A + len);
	auto iv = convert2pair(genInp(Av));
	test_intervals<NIntvGap>(iv);
}

TEST(intv, gap2) {
	const int len = 14;
	int A[len] = { 0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0 };
	vector<int> Av(A, A + len);
	auto iv = convert2pair(genInp(Av));
	test_intervals<NIntvGap>(iv);
}


TEST(intv, gap3) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = gen_density(len);
		auto iv = convert2pair(genInp(A));
		test_intervals<NIntvGap>(iv, i);
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}


TEST(intv, gap4) {
	int len = 1000;
	for (size_t i = 0; i < 100; ++i) {
		auto iv = gen_intv(len);
		test_intervals<NIntvGap>(iv, i);
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}


using namespace utils;

class QueryFixture {
public:
	QueryFixture() {
	}

	virtual void SetUp(const int32_t problemSetValue) {
		inp_len = 5000000;
		query_size = 5000;
		inp = gen_intv(inp_len, problemSetValue / 100.0);
		idxq.clear();
		idxq.reserve(query_size);
		for (unsigned int i = 0; i < query_size; ++i) {
			idxq.push_back(rand32() % inp_len);
		}
		posq.clear();
		posq.reserve(query_size);
		unsigned int maxt = inp.back().second + 1;
		for (unsigned int i = 0; i < query_size; ++i) {
			posq.push_back(rand32() % maxt);
		}

		NIntvBuilder ilenbd;
		NIntvGapBuilder gapbd;
		NIntvGroupBuilder grbd;
		for (unsigned int i = 0; i < inp.size(); ++i) {
			ilenbd.add(inp[i].first, inp[i].second);
			gapbd.add(inp[i].first, inp[i].second);
			grbd.add(inp[i].first, inp[i].second);
		}
		ilenbd.build(&ilen);
		gapbd.build(&gap);
		grbd.build(&group);
	}

	virtual void TearDown() {
		ilen.clear();
		gap.clear();
		group.clear();

		inp.clear();
		idxq.clear();
		posq.clear();
	}

	unsigned int inp_len, query_size;
	std::vector<std::pair<unsigned int, unsigned int> > inp;

	std::vector<unsigned int> idxq, posq;

	NIntv ilen;
	NIntvGroup group;
	NIntvGap gap;
};

void nintv_normal_access(QueryFixture * fix) {
	//cout << "test_normal" << endl;
	for (auto p : fix->idxq) {
		fix->ilen.int_startend(p);
	}
}

void nintv_group_access(QueryFixture * fix) {
	//cout << "test_group" << endl;
	for (auto p : fix->idxq) {
		fix->group.int_startend(p);
	}
}

void nintv_gap_access(QueryFixture * fix) {
	//cout << "test_gap" << endl;
	for (auto p : fix->idxq) {
		fix->gap.int_startend(p);
	}
}

void run_benchmark() {
	Benchmarker<QueryFixture> bm;
	std::vector<int> vals = { 1, 25, 50, 75, 100 };
	bm.n_samples = 5;
	bm.set_sizes(vals);
	bm.add("nintv_normal_access", nintv_normal_access, 5);
	bm.add("nintv_gap_access", nintv_gap_access, 5);
	bm.add("nintv_group_access", nintv_group_access, 5);
	
	bm.run_all();
	bm.report(0);
}


