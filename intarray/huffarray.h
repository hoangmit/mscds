#pragma once

#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <stdexcept>
#include <stdint.h>
#include "codec/huffman_code.h"
#include "sdarray_sml.h"
#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

namespace mscds {

class HuffmanBlk {
public:
	void clear();
	void mload(const BitArray * enc, const SDArraySml * ptr, unsigned startpos);
	void build(std::vector<uint32_t> * data, unsigned int subsize, OBitStream * out, std::vector<uint32_t> * opos);
	uint32_t lookup(unsigned int i) const;
	
private:
	coder::HuffmanCode hc;
	coder::HuffmanTree tc;
	std::vector<uint32_t> freq;
	std::map<uint32_t, uint32_t> freqset; //unordered_

	void buildModel(std::vector<uint32_t> * data);
	void saveModel(OBitStream * out);
	void loadModel(IWBitStream & is);

	void encode(uint32_t val, OBitStream * out);
	uint32_t decode(IWBitStream * is) const;

	const BitArray * enc;
	const SDArraySml * ptr;
	unsigned int subsize, len, pos;
};

//------------------------------------------------------------------------------

class HuffmanArray;

class HuffmanArrBuilder {
public:
	void init(unsigned int rate = 32, unsigned int secondrate=512) {
		rate1 = rate; rate2 = secondrate;
		cnt = 0;
	}
	void add(uint32_t val);
	void build(HuffmanArray * out);
	void clear() { buf.clear(); bd.clear(); rate1 = 0; rate2 = 0; opos.clear(); }
private:
	unsigned int rate1, rate2, cnt;
	HuffmanBlk blk;
	OBitStream out;
	std::vector<uint32_t> opos;
	void build_huffman(HuffmanArray * out);
	std::vector<uint32_t> buf;
	SDArraySmlBuilder bd;
};


class HuffmanArray {
public:
	HuffmanArray(): curblk(-1) {}
	void load(IArchive& ar);
	void save(OArchive& ar) const;
	void clear();
	uint32_t lookup(unsigned int i) const;
	uint32_t operator[](unsigned int pos) { return lookup(pos); }

	struct Enum: public EnumeratorInt<uint32_t> {
	public:
		Enum() {}
		Enum(const Enum& o): is(o.is) {}
		bool hasNext() const;
		uint32_t next();
	private:
		mscds::IWBitStream is;
		friend class HuffmanArray;
	};
	void getEnum(uint32_t pos, Enum * e) const;

private:
	mutable HuffmanBlk blk;
	mutable int curblk;
	SDArraySml ptr;
	BitArray bits;
	unsigned int len, rate1, rate2;
	
	friend class HuffmanArrBuilder;
};

}//namespace
