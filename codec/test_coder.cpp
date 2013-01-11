#include <stdio.h>

#include "deltacoder.h"
#include "golomb_coder.h"

#include "fibcoder.h"
#include "utils/str_utils.h"
#include "bitarray/bitop.h"

#include <cstdio>

#include "utils/utest.h"

#include <sstream>
#include <iostream>
#include <string>
#include <cassert>
#include <random>


using namespace std;
using namespace coder;
using namespace utils;
using namespace mscds;

void test_codestream_all();

void FibCoder_SmallNumbers() {
	ASSERT(binstr(FibCoder::encode(1)) == "11");
	ASSERT(binstr(FibCoder::encode(2)) == "011");
	ASSERT(binstr(FibCoder::encode(3)) == "0011");
	ASSERT(binstr(FibCoder::encode(4)) == "1011");

	ASSERT(binstr(FibCoder::encode(5)) == "00011");
	ASSERT(binstr(FibCoder::encode(6)) == "10011");
	ASSERT(binstr(FibCoder::encode(7)) == "01011");
	ASSERT(binstr(FibCoder::encode(8)) == "000011");

	ASSERT(binstr(FibCoder::encode(9)) == "100011");
	ASSERT(binstr(FibCoder::encode(10))== "010011");

	ASSERT(binstr(FibCoder::encode(52))  == "000101011");
	ASSERT(binstr(FibCoder::encode(65))  == "0100100011");
}

void FibCoder_DecodeEncode() {
	for (unsigned int i = 1; i < 1000000; ++i) {
		ASSERT_EQ(i, FibCoder::decode2(FibCoder::encode(i).first).first);
		ASSERT_EQ(FibCoder::encodelen(i), FibCoder::encode(i).second);
		ASSERT_EQ(FibCoder::encodelen(i), FibCoder::decode2(FibCoder::encode(i).first).second);
	}
}
//---------------------------------------------------------------------------
void GammaCoder_SmallNumbers() {
	ASSERT(binstr(GammaCoder::encode(1)) == "1");
	ASSERT(binstr(GammaCoder::encode(2)) == "010");
	ASSERT(binstr(GammaCoder::encode(3)) == "011");

	ASSERT(binstr(GammaCoder::encode(4)) == "00100");
	ASSERT(binstr(GammaCoder::encode(5)) == "00110");
	ASSERT(binstr(GammaCoder::encode(6)) == "00101");
	ASSERT(binstr(GammaCoder::encode(7)) == "00111");

	ASSERT(binstr(GammaCoder::encode(8)) == "0001000");
	ASSERT(binstr(GammaCoder::encode(9)) == "0001100");
	ASSERT(binstr(GammaCoder::encode(10))== "0001010");
}

void GammaCoder_DecodeEncode() {
	for (unsigned int i = 1; i < 1000000; ++i) {
		ASSERT_EQ(i, GammaCoder::decode2(GammaCoder::encode(i).first).first);
		ASSERT_EQ(GammaCoder::encodelen(i), GammaCoder::encode(i).second);
		ASSERT_EQ(GammaCoder::encodelen(i), GammaCoder::decode2(GammaCoder::encode(i).first).second);
	}
}

//---------------------------------------------------------------------------
void DeltaCoder_SmallNumbers() {
	ASSERT(binstr(DeltaCoder::encode(1)) == "1");
	ASSERT(binstr(DeltaCoder::encode(2)) == "0100");
	ASSERT(binstr(DeltaCoder::encode(3)) == "0101");

	ASSERT(binstr(DeltaCoder::encode(4)) == "01100");
	ASSERT(binstr(DeltaCoder::encode(5)) == "01110");
	ASSERT(binstr(DeltaCoder::encode(6)) == "01101");
	ASSERT(binstr(DeltaCoder::encode(7)) == "01111");

	ASSERT(binstr(DeltaCoder::encode(8)) == "00100000");
	ASSERT(binstr(DeltaCoder::encode(9)) == "00100100");
	ASSERT(binstr(DeltaCoder::encode(10))== "00100010");
	ASSERT(binstr(DeltaCoder::encode(11))== "00100110");
	ASSERT(binstr(DeltaCoder::encode(12))== "00100001");
	ASSERT(binstr(DeltaCoder::encode(13))== "00100101");
	ASSERT(binstr(DeltaCoder::encode(14))== "00100011");
	ASSERT(binstr(DeltaCoder::encode(15))== "00100111");
}

void DeltaCoder_DecodeEncode() {
	for (unsigned int i = 1; i < 1000000; ++i) {
		ASSERT(i == DeltaCoder::decode2(DeltaCoder::encode(i).first).first);
		ASSERT_EQ(DeltaCoder::encodelen(i), DeltaCoder::encode(i).second);
		ASSERT_EQ(DeltaCoder::encodelen(i), DeltaCoder::decode2(DeltaCoder::encode(i).first).second);
	}
}

void CeilLog_Simple() {
	ASSERT_EQ(0, ceillog2(0));
	ASSERT_EQ(0, ceillog2(1));
	ASSERT_EQ(1, ceillog2(2));
	ASSERT_EQ(2, ceillog2(3));
	ASSERT_EQ(2, ceillog2(4));
	ASSERT_EQ(3, ceillog2(5));
	ASSERT_EQ(3, ceillog2(6));
	ASSERT_EQ(3, ceillog2(7));
	ASSERT_EQ(3, ceillog2(8));
	ASSERT_EQ(4, ceillog2(9));
}

void RiceCoder_Encode1() {
	RiceCoder r0(0);
	ASSERT_EQ("0", binstr(r0.encode(0)));
	ASSERT_EQ("10", binstr(r0.encode(1)));
	ASSERT_EQ("110", binstr(r0.encode(2)));
	ASSERT_EQ("1110", binstr(r0.encode(3)));
	ASSERT_EQ("11110", binstr(r0.encode(4)));
	ASSERT_EQ("111110", binstr(r0.encode(5)));
	ASSERT_EQ("1111110", binstr(r0.encode(6)));
	
	RiceCoder r1(1);
	ASSERT_EQ(string("0")+"0", binstr(r1.encode(0)));
	ASSERT_EQ(string("0")+"1", binstr(r1.encode(1)));
	ASSERT_EQ(string("10")+"0", binstr(r1.encode(2)));
	ASSERT_EQ(string("10")+"1", binstr(r1.encode(3)));
	ASSERT_EQ(string("110")+"0", binstr(r1.encode(4)));
	ASSERT_EQ(string("110")+"1", binstr(r1.encode(5)));
	ASSERT_EQ(string("1110")+"0", binstr(r1.encode(6)));
	ASSERT_EQ(string("1110")+"1", binstr(r1.encode(7)));
	ASSERT_EQ(string("11110")+"0", binstr(r1.encode(8)));

	RiceCoder r2(2);
	ASSERT_EQ(string("0")+"00", binstr(r2.encode(0)));
	ASSERT_EQ(string("0")+"10", binstr(r2.encode(1)));
	ASSERT_EQ(string("0")+"01", binstr(r2.encode(2)));
	ASSERT_EQ(string("0")+"11", binstr(r2.encode(3)));
	ASSERT_EQ(string("10")+"00", binstr(r2.encode(4)));
	ASSERT_EQ(string("10")+"10", binstr(r2.encode(5)));
	ASSERT_EQ(string("10")+"01", binstr(r2.encode(6)));
	ASSERT_EQ(string("10")+"11", binstr(r2.encode(7)));
	ASSERT_EQ(string("110")+"00", binstr(r2.encode(8)));
	for (int i = 0; i < 5; ++i) {
		RiceCoder rx(i);
		for (int j = 0; j < 20; ++j) {
			ASSERT_EQ(j, rx.decode2(rx.encode(j).first).first); 
			ASSERT_EQ(rx.encodelen(j), rx.encode(j).second);
			ASSERT_EQ(rx.encodelen(j), rx.decode2(rx.encode(j).first).second);
		}
	}
}

void GolombCoder_Encode1() {
	GolombCoder g(10);
	ASSERT_EQ(string("0")+"000", binstr(g.encode(0)));
	ASSERT_EQ(string("0")+"100", binstr(g.encode(1)));
	ASSERT_EQ(string("0")+"010", binstr(g.encode(2)));
	ASSERT_EQ(string("0")+"110", binstr(g.encode(3)));
	ASSERT_EQ(string("0")+"001", binstr(g.encode(4)));
	ASSERT_EQ(string("0")+"101", binstr(g.encode(5)));
	ASSERT_EQ(string("0")+"0110", binstr(g.encode(6)));
	ASSERT_EQ(string("0")+"0111", binstr(g.encode(7)));
	ASSERT_EQ(string("0")+"1110", binstr(g.encode(8)));
	ASSERT_EQ(string("0")+"1111", binstr(g.encode(9)));
	ASSERT_EQ(string("10")+"000", binstr(g.encode(10)));
	for (int j = 0; j < 100; ++j)
		ASSERT_EQ(j, g.decode2(g.encode(j).first).first);
	g = GolombCoder(5);
	ASSERT_EQ(string("0")+"00", binstr(g.encode(0)));
	ASSERT_EQ(string("0")+"10", binstr(g.encode(1)));
	ASSERT_EQ(string("0")+"01", binstr(g.encode(2)));
	ASSERT_EQ(string("0")+"110", binstr(g.encode(3)));
	ASSERT_EQ(string("0")+"111", binstr(g.encode(4)));
	ASSERT_EQ(string("10")+"00", binstr(g.encode(5)));
	ASSERT_EQ(string("10")+"10", binstr(g.encode(6)));
	ASSERT_EQ(string("10")+"01", binstr(g.encode(7)));
	ASSERT_EQ(string("10")+"110", binstr(g.encode(8)));
	ASSERT_EQ(string("10")+"111", binstr(g.encode(9)));
	ASSERT_EQ(string("110")+"00", binstr(g.encode(10)));
	for (int j = 0; j < 50; ++j)
		ASSERT_EQ(j, g.decode2(g.encode(j).first).first);
	g = GolombCoder(35);
	ASSERT_EQ(string("0")+"00000", binstr(g.encode(0)));
	ASSERT_EQ(string("0")+"10000", binstr(g.encode(1)));
	ASSERT_EQ(string("0")+"10011", binstr(g.encode(25)));
	ASSERT_EQ(string("0")+"11011", binstr(g.encode(27)));
	ASSERT_EQ(string("0")+"00111", binstr(g.encode(28)));
	ASSERT_EQ(string("0")+"101110", binstr(g.encode(29)));
	ASSERT_EQ(string("0")+"101111", binstr(g.encode(30)));
	ASSERT_EQ(string("0")+"011110", binstr(g.encode(31)));
	ASSERT_EQ(string("0")+"011111", binstr(g.encode(32)));
	ASSERT_EQ(string("0")+"111110", binstr(g.encode(33)));
	ASSERT_EQ(string("0")+"111111", binstr(g.encode(34)));
	for (int j = 0; j < 150; ++j) {
		ASSERT_EQ(j, g.decode2(g.encode(j).first).first);
		ASSERT_EQ(g.encodelen(j), g.encode(j).second);
		ASSERT_EQ(g.encodelen(j),g.decode2(g.encode(j).first).second);
	}
}

void GolombAdpCoder_estimate1() {
	std::default_random_engine eng;
	double p = 0.9;
	std::geometric_distribution<int, double> geometric(p);
	int M = 0;
	//size_t tlen;
	int med = (int)ceil(-log(2.0)/ log(p));
	//eng.seed(37);
	AdpGolombCoder agc;
	for (int i = 0; i < 100000; i++) {
		int num;
		num = geometric(eng);
		assert(num >= 0);
		CodePr a = agc.encode(num);
	}
	ASSERT_EQ(med, agc.current_M());
}

void GolombAdpCoder_encode_decode1() {
	std::default_random_engine eng;
	double p = 0.9;
	std::geometric_distribution<int> geometric(p);
	int M = 0;
	//size_t tlen;
	int med = (int)ceil(-log(2.0)/ log(p));
	eng.seed(37);
	AdpGolombCoder agc, dgc;
	for (int i = 0; i < 100000; i++) {
		int num;
		num = geometric(eng);
		assert(num >= 0);
		CodePr a = agc.encode(num);
		CodePr b = dgc.decode(a.first);
		ASSERT_EQ(num, b.first);
	}
}


void GolombAdpCoder_encode_decode2() {
	std::mt19937 eng;
	double p = 0.9;
	std::geometric_distribution<int> geometric(p);
	int M = 0;
	//size_t tlen;
	int med = (int)ceil(-log(2.0)/ log(p));
	eng.seed(37);
	AdpGolombCoder agc, dgc;
	for (int i = 0; i < 100000; i++) {
		int num;
		num = geometric(eng) - 1;
		assert(num >= 0);
		CodePr a = agc.encode(num);
		CodePr b = dgc.decode(a.first);
		ASSERT_EQ(num, b.first);
		if (rand() % 1000 == 1) {
			int nx = rand()% 100 + 10000;
			CodePr a = agc.encode(nx);
			CodePr b = dgc.decode(a.first);
			ASSERT_EQ(nx, b.first);
		}
	}
}

//#include "bstreamg.hpp"
/*
int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "GolombAdpCoder.encode_decode*";
	::testing::GTEST_FLAG(break_on_failure) = true;
	::testing::InitGoogleTest(&argc, argv); 
	return RUN_ALL_TESTS(); 
}*/

int main(int argc, char* argv[]) {
	test_codestream_all();
	FibCoder_SmallNumbers();
	FibCoder_DecodeEncode();
	GammaCoder_SmallNumbers();
	GammaCoder_DecodeEncode();
	DeltaCoder_SmallNumbers();
	DeltaCoder_DecodeEncode();
	CeilLog_Simple();
	RiceCoder_Encode1();
	GolombCoder_Encode1();
	//GolombAdpCoder_estimate1();
	//GolombAdpCoder_encode_decode1();
	//GolombAdpCoder_encode_decode2();
	return 0;
}
