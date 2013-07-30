
#include <iostream>
#include "bitarray/bitstream.h"



void design_huffman() {
	/*
	CharStat stat;

	CharMapperStat stat2;

	for (data : xyz)
		stat.add(data);

	//--------------------------------

	OBitStream out;

	HuffmanStaticCoder coder(stat, out);

	for (data: xyz)
		coder.encode(data); //coder.add(data)


	HuffmanAdaptCoder coder2;
	*/
}

void design_arithmetic() {
	/*
	ArithmeticStream out;
	*/
}

#include "utils/utest.h"
#include <vector>

#include "huffman_code.h"
#include "huffman_adp.hpp"

using namespace std;
using namespace coder;


TEST(huffman,test1) {
	const int n = 5;
	int arr[n] = {6,5,3,3,1};
	vector<unsigned int> v;
	for (int i = 0; i < n; i++) v.push_back(arr[i]);
	HuffmanCode code;
	int tt = code.build(v);
	ASSERT_EQ(tt, 6*2 + 5*2 + 3*2 + 3*3 + 1*3);
}

TEST(huffman, encode_decode1) {
	const int n = 256;
	adp_huffman_enc enc(n);
	adp_huffman_dec dec(n);
	for (int i = 0; i < 100000; i++) {
		unsigned int v = (rand() % n);
		CodePr a = enc.encode(v);
		CodePr b = dec.decode(a.first);
		ASSERT_EQ(v, b.first);
		ASSERT_EQ(a.second, b.second);
	}
}

TEST(huffman, encode_decode2) {
	const int n = 256;
	adp_huffman_enc enc(n);
	adp_huffman_dec dec(n);
	for (int i = 0; i < 100000; i++) {
		unsigned int v = (rand() % 128);
		if (rand() % 3 < 2) v += (rand() % 128);
		CodePr a = enc.encode(v);
		CodePr b = dec.decode(a.first);
		ASSERT_EQ(v, b.first);
		ASSERT_EQ(a.second, b.second);
	}
}

TEST(huffman, canonical_code1) {
	const int n = 4;
	int arr[n] = {2, 1, 3, 3};
	vector<uint16_t> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = HuffmanCode::canonical_code(n, L);
	ASSERT_EQ(1, out[0]); // 11
	ASSERT_EQ(0, out[1]); // 0
	ASSERT_EQ(3, out[2]); // 101
	ASSERT_EQ(7, out[3]); // 100
}

TEST(huffman, canonical_code2) {
	const int n = 9;
	int arr[n] = {2, 3, 3, 3, 4, 4, 4, 5, 5};
	vector<uint16_t> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = HuffmanCode::canonical_code(n, L);
	ASSERT_EQ(0, out[0]);  // 00
	ASSERT_EQ(2, out[1]);  // 010
	ASSERT_EQ(6, out[2]);  // 011
	ASSERT_EQ(1, out[3]);  // 100
	ASSERT_EQ(5, out[4]);  // 1010
	ASSERT_EQ(13, out[5]); // 1011
	ASSERT_EQ(3, out[6]);  // 1100
	ASSERT_EQ(11, out[7]); // 11010
	ASSERT_EQ(27, out[8]); // 11011
}

TEST(huffman, canonical_code3) {
	const int n = 9;
	int arr[n] = {2, 2, 3, 3, 4, 4, 5, 5, 4};
	vector<uint16_t> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = HuffmanCode::canonical_code(n, L);
	ASSERT_EQ(0, out[0]);  // 00
	ASSERT_EQ(2, out[1]);  // 01
	ASSERT_EQ(1, out[2]);  // 100
	ASSERT_EQ(5, out[3]);  // 101

	ASSERT_EQ(3, out[4]);  // 1100
	ASSERT_EQ(11, out[5]); // 1101
	ASSERT_EQ(7, out[8]);  // 1110

	ASSERT_EQ(15, out[6]); // 11110
	ASSERT_EQ(31, out[7]); // 11111
}

#include "intarray/huffarray.h"

TEST(huffman, harray) {
	vector<uint32_t> A;
	unsigned int n = 50000;
	for (unsigned int i = 0; i < n; ++i)
		A.push_back(rand() % 20);

	mscds::HuffmanArrBuilder bd;
	bd.init();
	for (unsigned int i = 0; i < n; ++i)
		bd.add(A[i]);
	mscds::HuffmanArray arr;
	bd.build(&arr);
	for (unsigned int i = 0; i < n - 1; ++i) {
		ASSERT_EQ(A[i], arr.lookup(i));
	}
}


TEST(huffman, harray2) {
	vector<uint32_t> A;
	unsigned int n = 50000;
	for (unsigned int i = 0; i < n; ++i)
		A.push_back(rand() % 36);

	mscds::HuffmanArrBuilder bd;
	bd.init();
	for (unsigned int i = 0; i < n; ++i)
		bd.add(A[i]);
	mscds::HuffmanArray arr;
	bd.build(&arr);
	for (unsigned int i = 0; i < n - 1; ++i) {
		ASSERT_EQ(A[i], arr.lookup(i));
	}
}


int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}
