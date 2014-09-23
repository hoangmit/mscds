
#include "arithmetic_code.hpp"

#include "utils/utest.h"
#include <stdint.h>
#include <vector>
#include <algorithm>

using namespace coder;
using namespace std;


void test_bits(const vector<bool>& bv) {
	size_t zc = 0, oc = 0, n;
	n = bv.size();
	for (unsigned int i = 0; i < n; ++i) {
		if (bv[i]) oc++;
		else zc++;
	}
	OutBitStream::VecTp buf;
	OutBitStream is(&buf);
	AC32_EncState enc(&is);

	for (int i = 0; i < n; ++i) {
		if (!bv[i])
			enc.update(0, zc, n);
		else
			enc.update(zc, n, n);
	}
	enc.close();

	InBitStream os(is);
	AC32_DecState dec(&os);

	for (unsigned int i = 0; i < n; ++i) {
		unsigned int dc = dec.decode_count(n);
		bool val = dc >= zc;
		ASSERT_EQ(bv[i], val);
		if (bv[i] != val) {
			cout << "wrong";
			throw runtime_error("wrong");
		}
		if (val) dec.update(zc, n, n);
		else dec.update(0, zc, n);
	}
	dec.close();
}

void test_uint8(const vector<uint8_t>& bv) {
	size_t n = bv.size();
	unsigned int alp_cnt = 256;
	std::vector<size_t> cum_freq(alp_cnt + 1);
	for (unsigned int i = 0; i < cum_freq.size(); ++i)
		cum_freq[i] = 0;
	for (unsigned int i = 0; i < bv.size(); ++i)
		cum_freq[bv[i]] += 1;

	size_t sum = 0;
	for (unsigned int i = 0; i < cum_freq.size(); ++i) {
		size_t nx = sum + cum_freq[i];
		cum_freq[i] = sum;
		sum = nx;
	}
	size_t total = bv.size();
	ASSERT_EQ(cum_freq[alp_cnt], total);

	OutBitStream::VecTp buf;
	OutBitStream is(&buf);
	AC32_EncState enc(&is);

	for (unsigned int i = 0; i < n; ++i) {
		uint8_t val = bv[i];
		enc.update(cum_freq[val], cum_freq[val+1], total);
	}
	enc.close();

	InBitStream os(is);
	AC32_DecState dec(&os);
	for (unsigned int i = 0; i < n; ++i) {
		unsigned int dc = dec.decode_count(n);
		unsigned int idx = std::upper_bound(cum_freq.begin(), cum_freq.end(), dc) - cum_freq.begin();
		ASSERT(idx > 0 && idx <= alp_cnt);
		uint8_t val = idx - 1;
		ASSERT_EQ(bv[i], val);
		if (bv[i] != val) {
			cout << "wrong";
			throw runtime_error("wrong");
		}
		dec.update(cum_freq[val], cum_freq[val+1], total);
	}

	dec.close();
}

vector<bool> gen_bits(unsigned int n, double prob0 = 0.5) {
	vector<bool> out;
	out.resize(n);
	unsigned c0 = (unsigned) (prob0 * RAND_MAX);
	for (int i = 0; i < n; i++) {
		bool val = rand() < c0 == 0 ? false : true;
		out[i] = val;
	}
	return out;
}


TEST(arithmetic_code, binary1) {
	vector<bool> v = gen_bits(8000, 0.5);
	test_bits(v);
	v = gen_bits(8000, 0.2);
	test_bits(v);
	v = gen_bits(8000, 0.8);
	test_bits(v);
}

vector<uint8_t> gen_byte_oe(unsigned int n, double prob_even=0.5) {
	vector<uint8_t> out;
	out.resize(n);
	unsigned c0 = (unsigned)(prob_even * RAND_MAX);
	for (int i = 0; i < n; i++) {
		uint8_t val = rand() % 128;
		if (rand() < c0) val *= 2;
		else val = val * 2 + 1;
		out[i] = val;
	}
	return out;
}

TEST(arithmetic_code, char1) {
	vector<uint8_t> v = gen_byte_oe(8000, 0.5);
	test_uint8(v);
	v = gen_byte_oe(8000, 0.2);
	test_uint8(v);
	v = gen_byte_oe(8000, 0.8);
	test_uint8(v);
}

/*
int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(catch_exceptions) = "0";
	//::testing::GTEST_FLAG(break_on_failure) = "1";
	//::testing::GTEST_FLAG(filter) = "*.*";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;
}*/

