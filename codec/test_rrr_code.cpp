#include "rrr_codec.h"
#include "utils/utest.h"

#include <iostream>
using namespace std;

using namespace coder;

static void check(unsigned n, uint64_t w) {
	RRR_Codec rrr;
	unsigned k = mscds::popcnt(w);
	assert(k <= n);
	uint64_t ofs = rrr.encode(n, k, w);
	uint64_t dv = rrr.decode(n, k, ofs);
	if (w != dv) {
		std::cout << "Wrong " << w << " " << dv << std::endl;
		uint64_t ofs = rrr.encode(n, k, w);
		uint64_t dv = rrr.decode(n, k, ofs);
	}
	ASSERT_EQ(w, dv);
}

static void test_rnd(unsigned n, unsigned chance = 500) {
	uint64_t w = 0;
	for (unsigned i = 0; i < n; ++i) {
		w <<= 1;
		if (rand() % 1000 < 500) {
			w |= 1;
		}
	}
	check(n, w);
}

TEST(rrr_codec, test1) {
	for (unsigned i = 1; i <= 64; ++i) {
		check(i, 0);
		check(i, (~0ull) >> (64 - i));
	}

	for (unsigned int i = 0; i < 20000; ++i) {
		test_rnd(64);
		if (i % 400 == 0) cout << '.';
	}
	for (unsigned int i = 0; i < 20000; ++i) {
		test_rnd(32);
		if (i % 400 == 0) cout << '.';
	}
	for (unsigned int i = 0; i < 20000; ++i) {
		test_rnd(63);
		if (i % 400 == 0) cout << '.';
	}
	for (unsigned int i = 0; i < 20000; ++i) {
		test_rnd(31);
		if (i % 400 == 0) cout << '.';
	}
	std::cout << std::endl;
}

TEST(rrr_codec, test2) {
	for (unsigned int i = 0; i < 200000; ++i) {
		test_rnd(rand() % 64 + 1);
		if (i % 400 == 0) cout << '.';
	}
	std::cout << std::endl;
}


void test1() {
	
	RRR_Codec::_init_tables();
	for (unsigned i = 0; i <= 10; ++i) {
		std::cout << i << "> ";
		//std::cout << RRR_Codec::nCk[i*32 + i/2] << endl;

		for (unsigned j = 0; j <= i; ++j) 
			std::cout << (int) RRR_Codec::nCk_val(i,j) << '\t';
		std::cout << endl;
	}
}
