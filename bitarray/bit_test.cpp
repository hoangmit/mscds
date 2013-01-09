#include "bitarray.h"
#include "utils/str_utils.h"
#include "utils/utest.h"
#include "bitstream.h"
#include <vector>
#include <iostream>
#include <cassert>

using namespace std;
using namespace mscds;

void test_bitarr1(int len = 2048) {
	vector<bool> v(len);
	BitArray b = BitArray::create(len);
	for (int i = 0; i < v.size(); ++i) {
		v[i] = (rand() % 2) == 1;
		b.setbit(i, v[i]);
	}
	for (int i = 0; i < v.size(); ++i) { 
		ASSERT(v[i] == b[i]);
	}
	for (int i = 0; i < v.size() / 8; i++) {
		ASSERT(b.bits(i*8,8) == b.byte(i));
	}
	cout << "." ;
}

void test_bitarr2(int len = 2048) {
	vector<bool> v(len);
	BitArray b = BitArray::create(len);
	BitArray c = BitArray::create(len);
	for (int i = 0; i < v.size(); ++i) {
		v[i] = (rand() % 2) == 1;
		b.setbit(i, v[i]);
	}
	for (int i = 0; i < v.size(); ++i) { 
		ASSERT(v[i] == b[i]);
	}
	for (int d = 1; d <= 64; ++d) {
		for (int i = 0; i < v.size() - d; ++i) {
			uint64_t u = b.bits(i, d);
			uint64_t up = u;
			for (int j = i; j < i + d; j++) {
				ASSERT(v[j] == ((up & 1) > 0));
				up >>= 1;
			}
			bool bm1, bp1;
			if (i > 0) bm1 = c[i-1];
			if (i + d + 1 < v.size()) bp1 = c[i+d+1];
			c.setbits(i, u, d);
			if (u != c.bits(i,d)) {
				cout << u << ' ' << c.bits(i,d);
				c.setbits(i,u,d);
				ASSERT(u == c.bits(i,d));
			}
			if (i > 0) ASSERT(bm1 == c[i-1]);
			if (i + d + 1 < v.size()) bp1 = c[i+d+1];
		}
	}
	cout << "." ;
}

void test_obitstream(int len = 2048) {
	OBitStream os;
	vector<bool> v;
	for (int i = 0; i < len; ++i) {
		if (rand() % 2 == 1) {
			os.put1();
			v.push_back(true);
		}else {
			os.put0();
			v.push_back(false);
		}
	}
	os.close();
	BitArray b = BitArray::create(os.data_ptr(), len);
	for (int i = 0; i < len; ++i) {
		if (v[i] != b[i]) {
			cout << v[i] << " " << b[i] << endl;
			ASSERT_EQ(v[i], b[i]);
		}
	}
}

void test_ibitstream(int len, int idx) {
	OBitStream os;
	for (int i = 0; i < len; ++i)
		if (rand() % 2 == 1) os.put1();
		else os.put0();
	os.close();
	BitArray b = BitArray::create(os.data_ptr(), len);
	IWBitStream is(os.data_ptr(), 0, len);
	string s = os.to_str();
	int bl = len;
	int pos = 0, j = 0;
	while (pos < len) {
		int rl = rand() % 65;
		if (pos + rl > len) rl = len - pos;
		uint64_t exp = b.bits(pos, rl);
		uint64_t v = is.get(rl);
		ASSERT_EQ(exp, v);
		pos += rl;
		j++;
	}
	size_t x = is.current_ptr() - os.data_ptr();
	ASSERT_EQ(x, os.word_count());
	is.close();
}

void test_bit_all() {
	for (int i = 0; i < 500; i++)
		test_ibitstream(2048 + rand() % 64, i);

	for (int i = 0; i < 500; i++)
		test_obitstream(1024 + rand() % 64);

	for (int i = 0; i < 1000; i++)
		test_bitarr1(1024 + rand() % 256);
	for (int i = 0; i < 100; i++)
		test_bitarr2(512 + rand() % 256);
}

int main() {
	test_bit_all();
	return 0;
}