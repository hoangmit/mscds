#include "bitarray.h"
#include "utils/str_utils.h"
#include <vector>
#include <iostream>
#include <cassert>

using namespace std;
using namespace mscds;

void test1(int len = 2048) {
	vector<bool> v(len);
	BitArray b = BitArray::create(len);
	for (int i = 0; i < v.size(); ++i) {
		v[i] = (rand() % 2) == 1;
		b.setbit(i, v[i]);
	}
	for (int i = 0; i < v.size(); ++i) { 
		assert(v[i] == b.bit(i));
	}
	cout << "." ;
}

void test2(int len = 2048) {
	vector<bool> v(len);
	BitArray b = BitArray::create(len);
	BitArray c = BitArray::create(len);
	for (int i = 0; i < v.size(); ++i) {
		v[i] = (rand() % 2) == 1;
		b.setbit(i, v[i]);
	}
	for (int i = 0; i < v.size(); ++i) { 
		assert(v[i] == b.bit(i));
	}
	for (int d = 1; d <= 64; ++d) {
		for (int i = 0; i < v.size() - d; ++i) {
			uint64_t u = b.bits(i, d);
			uint64_t up = u;
			for (int j = i; j < i + d; j++) {
				assert(v[j] == ((up & 1) > 0));
				up >>= 1;
			}
			bool bm1, bp1;
			if (i > 0) bm1 = c.bit(i-1);
			if (i + d + 1 < v.size()) bp1 = c.bit(i+d+1);
			c.setbits(i, u, d);
			if (u != c.bits(i,d)) {
				cout << u << ' ' << c.bits(i,d);
				c.setbits(i,u,d);
				assert(u == c.bits(i,d));
			}
			if (i > 0) assert(bm1 == c.bit(i-1));
			if (i + d + 1 < v.size()) bp1 = c.bit(i+d+1);
		}
	}
	cout << "." ;
}

void test_bit_all() {
	for (int i = 0; i < 100; i++)
		test2(512 + rand() % 256);
	for (int i = 0; i < 1000; i++)
		test1(1024 + rand() % 256);
}

int main() {
	test_bit_all();
	return 0;
}