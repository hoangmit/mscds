#include "bstreamg.hpp"
#include "deltacoder.h"
#include "golomb_coder.h"

#include <iostream>

#include "utils/utest.h"
using namespace std;
using namespace coder;
/*
void EncBStream_delta_EncodeSmall1(){
	VecOStream::VecTp store;
	VecOStream v(store);
	EncWStream<VecOStream, DeltaCoder> e(v);
	for (int i = 1; i <= 10; ++i) {
		e.push(i);
	}
	e.stop();
	// 1 0100 0101 01100 01110 01101 01111 00100000 00100100 00100010
	string s = v.to_str();
	ASSERT_EQ(s, "10100010101100011100110101111001000000010010000100010");
}

void EncBStream_delta_EncodeSmall2() {
	VecOStream::VecTp store;
	VecOStream v(store);
	EncWStream<VecOStream, DeltaCoder> e(v);
	for (int i = 1; i <= 14; ++i) {
		e.push(i);
	}
	e.stop();
	// 1 0100 0101 01100 01110 01101 01111 00100000 00100100 00100010 00100110 00100001 00100101 00100011
	ASSERT(v.to_str() == "1010001010110001110011010111100100000001001000010001000100110001000010010010100100011");
}

void EncBStream_delta_encodedecode_small1()  {
	VecOStream::VecTp store;
	VecOStream v(store);
	EncWStream<VecOStream, DeltaCoder> e(v);
	int i;
	for (i = 1; i <= 10; ++i) {
		e.push(i);
		//cout << store[0] << " " << store[1] << "   " << v.bitlength() << endl;
	}
	e.stop();
	// Decode
	VecIStream v2(store, v.blen);
	DecI64Stream<VecIStream, DeltaCoder> d(v2);
	i = 0;
	while (!d.empty()) {
		uint64_t v = d.pull();
		++i;
		ASSERT_EQ(i, v);
	}
	ASSERT_EQ(10, i);
}

void EncBStream_delta_encodedecode_small2()  {
	VecOStream::VecTp store;
	VecOStream v(store);
	EncWStream<VecOStream, DeltaCoder> e(v);
	int i;
	for (i = 1; i <= 14; ++i) {
		e.push(i);
	}
	e.stop();
	// Decode
	VecIStream v2(store, v.blen);
	DecI64Stream<VecIStream, DeltaCoder> d(v2);
	i = 0;
	while (!d.empty()) {
		uint64_t v = d.pull();
		++i;
		ASSERT_EQ(i, v);
	}
	ASSERT_EQ(14, i);
}

void EncBStream_delta_encodedecode_medium()  {
	VecOStream::VecTp store;
	VecOStream v(store);
	EncWStream<VecOStream, DeltaCoder> e(v);
	int i;
	for (i = 1; i <= 100000; ++i) {
		e.push(i);
	}
	e.stop();
	// Decode
	VecIStream v2(store, v.blen);
	DecI64Stream<VecIStream, DeltaCoder> d(v2);
	i = 0;
	while (!d.empty()) {
		uint64_t v = d.pull();
		++i;
		ASSERT_EQ(i, v);
	}
	ASSERT_EQ(100000, i);
}

void EncBStream_golomb_encodedecode_small2()  {
	VecOStream::VecTp store;
	VecOStream v(store);
	GolombCoder g(10);
	EncWStream<VecOStream, GolombCoder> e(v,g);
	int i;
	for (i = 1; i <= 100; ++i) {
		e.push(i);
	}
	e.stop();
	// Decode
	VecIStream v2(store, v.blen);
	DecI64Stream<VecIStream, GolombCoder> d(v2,g);
	i = 0;
	while (!d.empty()) {
		uint64_t v = d.pull();
		++i;
		ASSERT_EQ(i, v);
	}
	ASSERT_EQ(100, i);
}


*/