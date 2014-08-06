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
	BitArray b = BitArrayBuilder::create(len);
	for (int i = 0; i < v.size(); ++i) {
		v[i] = (rand() % 2) == 1;
		b.setbit(i, v[i]);
	}
	for (int i = 0; i < v.size(); ++i) { 
		ASSERT_EQ(v[i], b[i]);
	}
	for (int i = 0; i < v.size() / 8; i++) {
		ASSERT_EQ(b.bits(i*8,8), b.byte(i));
	}
}

void test_bitarr2(int len = 2048) {
	vector<bool> v(len);
	BitArray b = BitArrayBuilder::create(len);
	BitArray c = BitArrayBuilder::create(len);
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
	BitArray b;
	os.build(&b);
	for (int i = 0; i < len; ++i) {
		if (v[i] != b[i]) {
			cout << v[i] << " " << b[i] << endl;
			ASSERT_EQ(v[i], b[i]);
		}
	}
}

void test_ibitstream(int len, int idx) {
	OBitStream os;
	string debug;
	for (int i = 0; i < len; ++i)
		if (rand() % 2 == 1) { os.put1(); debug.append("1"); }
		else { os.put0();  debug.append("0"); }
	os.close();
	BitArray b;
	string s = os.to_str();
	ASSERT_EQ(debug, s);
	IWBitStream is(os);
	os.build(&b);
	
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
	ASSERT_EQ(true, is.empty());
	//size_t x = is.current_ptr() - os.data_ptr();
	//ASSERT_EQ(x, os.word_count());
	is.close();
}

void test_append(int l1, int l2) {
	OBitStream o1, o2;
	string debug;
	for (int i = 0; i < l1; ++i)
		if (rand() % 2 == 1) { o1.put1(); debug.append("1"); }
		else { o1.put0();  debug.append("0"); }
	for (int i = 0; i < l2; ++i)
		if (rand() % 2 == 1) { o2.put1(); debug.append("1"); }
		else { o2.put0();  debug.append("0"); }
	o2.close();
	o1.append(o2);
	o1.close();
	string ost = o1.to_str();
	ASSERT_EQ(debug, ost);
}


void test_put_scan1(unsigned int n, unsigned int range) {
	std::vector<unsigned> runlen(n);
	for (unsigned i = 0; i < n; ++i) {
		runlen[i] = rand() % range;
	}
	OBitStream out;
	for (unsigned i = 0; i < n; ++i) {
		out.put0(runlen[i]);
		out.put1();
	}
	out.close();
	IWBitStream in(out);
	for (unsigned i = 0; i < n; ++i) {
		unsigned int l = in.scan_next1();
		ASSERT_EQ(runlen[i], l);
	}
}

void test_put_scan0(unsigned int n, unsigned int range) {
	std::vector<unsigned> runlen(n);
	for (unsigned i = 0; i < n; ++i) {
		runlen[i] = rand() % range;
	}
	OBitStream out;
	for (unsigned i = 0; i < n; ++i) {
		out.put1(runlen[i]);
		out.put0();
	}
	out.close();
	IWBitStream in(out);
	for (unsigned i = 0; i < n; ++i) {
		ASSERT_EQ(runlen[i], in.scan_next0());
	}
}

TEST(bitstream, inp) {
	for (int i = 0; i < 1000; i++) {
		test_ibitstream(2044 + rand() % 64, i);
		if (i % 100 == 0) cout << '.';
	}
	cout << endl;
}

TEST(bitstream, bitarray) {
	for (int i = 0; i < 1000; i++) {
		test_bitarr1(1022 + rand() % 256);
		if (i % 100 == 0) cout << '.';
	}
	for (int i = 0; i < 100; i++) {
		test_bitarr2(510 + rand() % 256);
		if (i % 100 == 0) cout << '.';
	}
	cout << endl;
}

TEST(bitstream, out) {
	for (int i = 0; i < 500; i++) {
		test_obitstream(1022 + rand() % 64);
		if (i % 100 == 0) cout << '.';
	}
	cout << endl;
}

TEST(bitstream, append) {
	for (int i = 0; i < 129; ++i)
		for (int j = 0; j < 129; ++j)
			test_append(i, j);
}

TEST(bitstream, scan) {
	for (int i = 0; i < 1000; ++i) {
		test_put_scan1(rand() % 8 + 125, 10);
		test_put_scan0(rand() % 8 + 125, 68);
		test_put_scan1(rand() % 8 + 64*8*3 - 2, 66);
	}
}

BitArray gen_bits(unsigned int len) {
	OBitStream o;
	for (unsigned int i = 0; i < len; ++i) {
		if (rand() % 2 == 1) o.put1();
		else o.put0();
	}
	o.close();
	BitArray b;
	o.build(&b);
	return b;
}

TEST(bitarray, scan1_fit) {
	for (unsigned int i = 0; i < 100; i++) {
		unsigned int len = 128;
		BitArray b = gen_bits(len);
		for (unsigned int j = 0; j < len; ++j) {
			auto p = rand() % b.count_one();
			auto v1 = b.scan_bits(j, p);
			auto v2 = b.scan_bits_slow(j, p);
			ASSERT_EQ(v2, v1);
		}
	}
}

TEST(bitarray, scan1_unfit) {
	for (unsigned int i = 0; i < 100; i++) {
		unsigned int len = 64 + rand() % 64;
		BitArray b = gen_bits(len);
		for (unsigned int j = 0; j < len; ++j) {
			auto p = rand() % b.count_one();
			auto v1 = b.scan_bits(j, p);
			auto v2 = b.scan_bits_slow(j, p);
			ASSERT_EQ(v2, v1);
		}
	}
}

TEST(bitarray, scan0_fit) {
	for (unsigned int i = 0; i < 100; i++) {
		unsigned int len = 128;
		BitArray b = gen_bits(len);
		for (unsigned int j = 0; j < len; ++j) {
			auto p = rand() % b.count_one();
			auto v1 = b.scan_zeros(j, p);
			auto v2 = b.scan_zeros_slow(j, p);
			ASSERT_EQ(v2, v1);
		}
	}
}

TEST(bitarray, scan0_unfit) {
	for (unsigned int i = 0; i < 100; i++) {
		unsigned int len = 64 + rand() % 64;
		BitArray b = gen_bits(len);
		for (unsigned int j = 0; j < len; ++j) {
			auto p = rand() % b.count_one();
			auto v1 = b.scan_zeros(j, p);
			auto v2 = b.scan_zeros_slow(j, p);
			ASSERT_EQ(v2, v1);
		}
	}
}



//TESTALL_MAIN();
