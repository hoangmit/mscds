
#include "utils/utest.h"
#include "nintv_fuse.h"
#include "intv_rand_gen.hpp"
using namespace mscds;
using namespace std;
using namespace app_ds;


template<typename T>
void test_intervals_easy(const std::vector<std::pair<unsigned int, unsigned int> >& rng, const T& r, int testid = 0) {
	ASSERT_EQ(rng.size(), r.length());
	size_t pslen = 0;
	for (size_t i = 0; i < rng.size(); ++i) {
		ASSERT_EQ(rng[i].first, r.int_start(i)) << "i == " << i;
		auto len = rng[i].second - rng[i].first;
		ASSERT_EQ(len, r.int_len(i)) << "i = " << i << std::endl;
		ASSERT_EQ(rng[i].second, r.int_end(i));
		ASSERT_EQ(pslen, r.int_psrlen(i)) << "i = " << i;
		pslen += len;
		auto px = r.int_startend(i);
		ASSERT(rng[i].first == px.first && rng[i].second == px.second);
	}
}

template<typename T>
void rank_cov(const std::vector<std::pair<unsigned int, unsigned int> >& rng, const T& r, int testid = 0) {
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
		} else {
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(0, p.second);
		}
	}
	ASSERT_EQ(rng.size() - 1, j);
	auto v = r.rank_interval(mlen);
	ASSERT_EQ(rng.size() - 1, v);
}

void test1() {
	vector<int> Av = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	auto rng = convert2pair(genInp(Av));
	
	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test2() {
	vector<int> Av = {1, 1, 1, 0, 9, 9, 0, 2, 2, 2, 3};
	auto rng = convert2pair(genInp(Av));

	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test3(unsigned int len = 1024) {
	auto rng = gen_intv2(1024, 50, 50);
	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test4() {
	for (unsigned i = 0; i < 50; ++i) 
		test3(1024);
	for (unsigned i = 0; i < 30; ++i)
		test3(1025);
	for (unsigned i = 0; i < 20; ++i)
		test3(1023);
}

int main() {
	test1();
	test2();
	test3();
	//test4();
	return 0; 
}

