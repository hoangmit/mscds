
#include "deltacoder.h"
#include "golomb_coder.h"
#include "bitarray/bitstream.h"

#include <iostream>

#include "utils/utest.h"

namespace tests {

using namespace std;
using namespace coder;
using namespace mscds;

TEST(EncBStream, delta_EncodeSmall1) {
	OBitStream os;
	DeltaCoder dc;
	for (int i = 1; i <= 10; ++i) {
		os.puts(dc.encode(i));
	}
	os.close();
	// 1 0100 0101 01100 01110 01101 01111 00100000 00100100 00100010
	string s = os.to_str();
	ASSERT_EQ(s, "10100010101100011100110101111001000000010010000100010");
}

TEST(EncBStream, delta_EncodeSmall2) {
	OBitStream os;
	DeltaCoder dc;
	for (int i = 1; i <= 14; ++i) {
		os.puts(dc.encode(i));
	}
	os.close();
	// 1 0100 0101 01100 01110 01101 01111 00100000 00100100 00100010 00100110 00100001 00100101 00100011
	ASSERT(os.to_str() == "1010001010110001110011010111100100000001001000010001000100110001000010010010100100011");
}

TEST(EncBStream, delta_encodedecode_small1) {
	OBitStream os;
	DeltaCoder dc;
	int i;
	for (i = 1; i <= 10; ++i) {
		os.puts(dc.encode(i));
	}
	os.close();
	// Decode
	IWBitStream is(os);
	i = 0;
	CodePr c;
	while (!is.empty()) {
		uint64_t v = is.peek();
		c = dc.decode2(v);
		is.skipw(c.second);
		++i;
		ASSERT_EQ(i, c.first);
	}
	ASSERT_EQ(10, c.first);
}

TEST(EncBStream, delta_encodedecode_small2) {
	OBitStream os;
	DeltaCoder dc;
	int i;
	for (i = 1; i <= 14; ++i) {
		os.puts(dc.encode(i));
	}
	os.close();
	// Decode
	IWBitStream is(os);
	i = 0;
	CodePr c;
	while (!is.empty()) {
		uint64_t v = is.peek();
		c = dc.decode2(v);
		is.skipw(c.second);
		++i;
		ASSERT_EQ(i, c.first);
	}
	ASSERT_EQ(14, c.first);
}

TEST(EncBStream, delta_encodedecode_medium) {
	OBitStream os;
	DeltaCoder dc;
	int i;
	for (i = 1; i <= 100000; ++i) {
		os.puts(dc.encode(i));
	}
	os.close();
	// Decode
	IWBitStream is(os);
	i = 0;
	CodePr c;
	while (!is.empty()) {
		uint64_t v = is.peek();
		c = dc.decode2(v);
		is.skipw(c.second);
		++i;
		ASSERT_EQ(i, c.first);
	}
	ASSERT_EQ(100000, c.first);
}


TEST(EncBStream, delta_encodedecode_rand) {
	OBitStream os;
	DeltaCoder dc;
	int i;
	vector<unsigned int> vals;
	for (i = 1; i <= 100000; ++i) {
		vals.push_back((rand() % 10000) + 1);
		os.puts(dc.encode(vals.back()));
	}
	os.close();
	// Decode
	IWBitStream is(os);
	i = 0;
	CodePr c;
	while (!is.empty()) {
		uint64_t v = is.peek();
		c = dc.decode2(v);
		is.skipw(c.second);
		ASSERT(i < vals.size());
		ASSERT_EQ(vals[i], c.first);
		++i;
	}
}


/*
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

TEST(EncBStream, seek_decode1) {
	size_t len = 100;
	vector<size_t> stpos;
	stpos.resize(len + 2);
	OBitStream os;
	DeltaCoder dc;
	for (size_t i = 1; i <= len; ++i) {
		CodePr c = dc.encode(i);
		os.puts(c);
		stpos[i] = c.second;
	}
	os.close();
	stpos[0] = 0;
	for (size_t i = 1; i <= len; ++i) {
		stpos[i] += stpos[i-1];
	}
	BitArray b;
	os.build(&b);
	IWBitStream is;
	for (size_t i = 0; i < len; ++i) {
		is.init(b, stpos[i]);
		CodePr c = dc.decode2(is.peek());
		ASSERT_EQ(i + 1, c.first);
	}
}

}//namespace
