
#include "codec/aux_codec.h"
#include "utils/utest.h"

#include <stdint.h>
#include <iostream>
#include <ctime>

using namespace std;
using namespace coder;

std::vector<uint8_t> gen_vec(unsigned int n, unsigned int range = 10) {
	std::vector<uint8_t> v;
	for (unsigned int i = 0; i < n; ++i) {
		v.push_back(rand() % range);
	}
	return v;
}

void test_mtf(const std::vector<uint8_t> & vec) {
	typedef DequeStream<uint8_t> IOStream;
	IOStream s1, s2;
	MTFEnc<uint8_t, IOStream, IOStream > encmtf;
	encmtf.bind(&s1, &s2, 8);
	unsigned int i  = 0;
	for (auto v: vec) {
		encmtf.add(v);
	}
	encmtf.close();
	cout << "decoding ... " << endl;
	i = 0;
	MTFDec<uint8_t, IOStream, IOStream> decmtf;
	decmtf.bind(&s1, &s2, 8);
	clock_t start = std::clock();
	for (auto v: vec) {
		uint8_t vx = decmtf.get();
		ASSERT_EQ(vx, v);
	}
	clock_t end = std::clock();
	double et = ((end - start)*1.0) / CLOCKS_PER_SEC;
	cout << "MTF decompression" << endl;
	cout << "time = " << et << "  (s)" << endl;
	cout << "through put = " << vec.size() / (et * 1024 * 1024) << "  MiB/s" << endl;
}

void test_rl(const std::vector<uint8_t> & vec) {
	typedef DequeStream<uint8_t> IOStream;
	IOStream s1, s2;
	RunLenEnc<uint8_t, IOStream, IOStream> encrl;
	encrl.bind(&s1, &s2);
	unsigned int i  = 0;
	for (auto v: vec) {
		//cout << i++ << " " << int(v) << endl;
		encrl.add(v);
	}
	encrl.close();
	cout << "decoding ... " << endl;
	i = 0;
	RunLenDec<uint8_t, IOStream, IOStream > decrl;
	decrl.bind(&s1, &s2);
	clock_t start = std::clock();
	for (auto v: vec) {
		//cout << i++ << endl;
		uint8_t vx = decrl.get();
		ASSERT_EQ(vx, v);
	}
	clock_t end = std::clock();
	double et = ((end - start)*1.0) / CLOCKS_PER_SEC;
	cout << "Runlen decompression" << endl;
	cout << "time = " << et << "  (s)" << endl;
	cout << "through put = " <<  vec.size() / (et * 1024 * 1024) << "  MiB/s" << endl;
}

TEST(MTF_RL_, test_1) {
	auto v = gen_vec(20000000, 20);
	test_mtf(v);
	test_rl(v);
}
