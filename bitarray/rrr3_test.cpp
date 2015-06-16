#include <iostream>
#include "utils/utest.h"
#include "mem/info_archive.h"
#include "rrr3.h"

#include <stdint.h>

using namespace std;
using namespace mscds;

static uint64_t randword() {
	uint64_t v = 0;
	for (unsigned i = 0; i < 8; ++i) {
		v <<= 8;
		v |= rand() % 256;
	}
	return v;
}

static uint64_t randword2(unsigned chance=500) {
	uint64_t w = 0;
	for (unsigned i = 0; i < 64; ++i) {
		w <<= 1;
		if (rand() % 1000 < chance) {
			w |= 1;
		}
	}
	return w;
}

static std::vector<uint64_t> gen0(unsigned len, unsigned mix = 0) {
	std::vector<uint64_t> ret;
	for (unsigned i = 0; i < len; ++i)
		ret.push_back(0);
	for (unsigned i = 0; i < mix; ++i) {
		ret[rand() % len] = randword();
	}	
	return ret;
}

static std::vector<uint64_t> gen1(unsigned len, unsigned mix = 0) {
	std::vector<uint64_t> ret;
	for (unsigned i = 0; i < len; ++i)
		ret.push_back(~0ull);
	for (unsigned i = 0; i < mix; ++i) {
		unsigned p = rand() % len;
		ret[p] = randword();
	}
	return ret;
}

static std::vector<uint64_t> gen_rnd(unsigned len) {
	std::vector<uint64_t> ret;
	for (unsigned i = 0; i < len; ++i) ret.push_back(randword());
	return ret;
}

static std::vector<uint64_t> gen_rnd2(unsigned len, unsigned chance) {
	std::vector<uint64_t> ret;
	for (unsigned i = 0; i < len; ++i) ret.push_back(randword2(chance));
	return ret;
}

template<typename WordAccess>
static void check(const std::vector<uint64_t>& vals) {
	typename WordAccess::BuilderTp bd;
	for (unsigned i = 0; i < vals.size(); ++i) {
		bd.add(vals[i]);
	}
	WordAccess qs;
	bd.build(&qs);
	for (unsigned i = 0; i < vals.size(); ++i) {
		unsigned v = qs.popcntw(i), exp = popcnt(vals[i]);
		if (v != exp) {
			cout << "Wrong" << endl;
			qs.popcntw(i);
		}
		ASSERT_EQ(exp, v);
	}
	for (unsigned i = 0; i < vals.size(); ++i) {
		uint64_t v = qs.getword(i), exp = vals[i];
		if (v != exp) {
			cout << "Wrong" << endl;
			qs.getword(i);
		}
		ASSERT_EQ(exp, v);
	}
	//int d = (int)(vals.size() * 64) - (int)(mscds::estimate_data_size(qs) * 8);
	//cout << d  << "  " << ((double)d)/(vals.size() * 64) << endl;
}

TEST(rrr_word, test1) {
	check<RRR_WordAccess>(gen1(2001, 5));
	check<RRR_WordAccess>(gen1(2001));
	check<RRR_WordAccess>(gen0(2001));
	check<RRR_WordAccess>(gen0(2001, 5));
}

TEST(rrr_word, test_rnd1) {
	for (unsigned i = 0; i < 200; ++i) {
		check<RRR_WordAccess>(gen_rnd(2000 + rand() % 100));
		check<RRR_WordAccess>(gen_rnd2(2000 + rand() % 100, 50));
		check<RRR_WordAccess>(gen_rnd2(2000 + rand() % 100, 950));
		if (i % 20 == 0) cout << '.';
	}
	cout << endl;
}

TEST(rrr_word, test2) {
	check<AdaptiveWordAccesss>(gen1(2001, 5));
	check<AdaptiveWordAccesss>(gen1(2001));
	check<AdaptiveWordAccesss>(gen0(2001));
	check<AdaptiveWordAccesss>(gen0(2001, 5));
}

TEST(rrr_word, test_rnd2) {
	for (unsigned i = 0; i < 200; ++i) {
		check<AdaptiveWordAccesss>(gen_rnd(2000 + rand() % 100));
		check<AdaptiveWordAccesss>(gen_rnd2(2000 + rand() % 100, 50));
		check<AdaptiveWordAccesss>(gen_rnd2(2000 + rand() % 100, 950));
		if (i % 20 == 0) cout << '.';
	}
	cout << endl;
}


/*
int main(int argc, char* argv[]) {
	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
	return 0;
}*/
