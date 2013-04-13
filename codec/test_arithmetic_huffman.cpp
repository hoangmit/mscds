#include "utils/utest.h"
#include <vector>
#include "arithmetic_code.hpp"
#include "huffman_code.hpp"

using namespace std;
using namespace coder;

void DISABLED_arithmetic_code_test1() {
	vector<bool> bv;
	int n = 5000, zc = 0, oc = 0;
	for (int i = 0; i < n; i++) {
		bool val = rand() % 2 == 0 ? false : true;
		if (val) oc++;
		else zc++;
		bv.push_back(val);
	}
	OutBitStream::VecTp buf;
	OutBitStream is(buf);
	AC32_EncState enc;
	enc.output = & is;
	enc.init();
	for (int i = 0; i < n; ++i) {
		//cout << (bv[i] ? "true  " : "false ");
		//cout <<  enc.lo << " " << enc.hi << ' ';
		if (!bv[i])
			enc.update(0, zc, n);
		else 
			enc.update(zc, n, n);
		//cout << endl;
	}
	enc.close();

	InBitStream os(is);
	AC32_DecState dec;
	dec.input = & os;
	dec.init();
	for (int i = 0; i < n; ++i) {
		//cout << dec.lo << " " << dec.hi << " " << dec.code << "   ";		
		unsigned int dc = dec.decode_count(n);
		bool val = dc >= zc;
		//cout << (val ? " true  " : " false ");
		ASSERT_EQ(bv[i], val);
		if (bv[i] != val) {
			cout << "wrong";
			throw runtime_error("wrong");
		}
		if (val) dec.update(zc, n, n);
		else dec.update(0, zc, n);
		//cout << endl;
	}
	dec.close();
}


TEST(test1,huffman) {
	const int n = 5;
	int arr[n] = {6,5,3,3,1};
	vector<int> v;
	for (int i = 0; i < n; i++) v.push_back(arr[i]);
	vector<pair<unsigned int, unsigned int> > output;
	int tt = huffman<int, int>(v, output);
	ASSERT_EQ(tt, 6*2 + 5*2 + 3*2 + 3*3 + 1*3);
}


TEST(canonical_code1,huffman) {
	const int n = 4;
	int arr[n] = {2, 1, 3, 3};
	vector<unsigned int> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = canonical_code(n, L);
	assert(out[0] == 1);
	assert(out[1] == 0);
	assert(out[2] == 3);
	assert(out[3] == 7);
}

TEST(encode_decode1,huffman) {
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

TEST(encode_decode2,huffman) {
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

/*
TEST(huffman, canonical_code2) {
	const int n = 5;
	int arr[n] = {2, 2, 2, 3, 3};
	vector<unsigned int> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = canonical_code(n, L);
	assert(out[0] == 1);
	assert(out[1] == 0);
	assert(out[2] == 3);
	assert(out[3] == 7);
}

TEST(huffman, canonical_code2) {
	const int n = 9;
	int arr[n] = {2, 2, 3, 3, 4, 4, 5, 5, 4};
	vector<unsigned int> L;
	for (int i = 0; i < n; i++) L.push_back(arr[i]);
	vector<unsigned int> out = canonical_code(n, L);
	assert(out[0] == 3);
	assert(out[1] == 0);
	assert(out[1] == 5);
	assert(out[1] == 4);
}*/
