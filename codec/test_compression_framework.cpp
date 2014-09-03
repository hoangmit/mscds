
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

#include "intarray/huffarray.h"

namespace tests {
using namespace std;
using namespace coder;


TEST(huffman, encode_decode3) {
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

TEST(huffman, encode_decode4) {
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

}//namespace

/*
int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}*/
