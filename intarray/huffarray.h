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

class HuffmanModel {
public:
	void buildModel(std::vector<uint32_t> * data);
	void saveModel(OBitStream * out) const;
	void loadModel(IWBitStream & is, bool decode_only = false);

	void clear();

	void encode(uint32_t val, OBitStream * out) const;
	uint32_t decode(IWBitStream * is) const;
private:
	coder::HuffmanCode hc;
	coder::HuffmanTree tc;
	std::vector<uint32_t> freq;
	std::map<uint32_t, uint32_t> freqset; //unordered_
};

class HuffmanBlk {
public:
	void clear();
	void mload(const BitArray * enc, const SDArraySml * ptr, unsigned startpos);
	void build(std::vector<uint32_t> * data, unsigned int subsize, OBitStream * out, std::vector<uint32_t> * opos);
	uint32_t lookup(unsigned int i) const;

	const HuffmanModel& getModel() const;
private:
	HuffmanModel model;
	const BitArray * enc;
	const SDArraySml * ptr;
	unsigned int subsize, len, blkpos;
};

//------------------------------------------------------------------------------

class HuffmanArray;

class HuffmanArrBuilder {
public:
	HuffmanArrBuilder();
	void init(unsigned int rate = 64, unsigned int secondrate=512);
	void add(uint32_t val);
	void build(OArchive& ar);
	void build(HuffmanArray * out);
	void clear();
	typedef HuffmanArray QueryTp;
private:
	unsigned int rate1, rate2, cnt;
	HuffmanBlk blk;
	OBitStream out;
	std::vector<uint32_t> opos;
	std::vector<uint32_t> buf;
	SDArraySmlBuilder bd;
};


class HuffmanArray {
public:
	HuffmanArray(): curblk(-1) {}
	void load(IArchive& ar);
	void save(OArchive& ar) const;
	void clear();
	uint64_t length() const;
	uint32_t lookup(unsigned int i) const;
	uint32_t operator[](unsigned int pos) { return lookup(pos); }

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum(): curblk(-1) {}
		bool hasNext() const { return pos < data->len;}
		uint64_t next();
	private:
		unsigned int pos;
		IWBitStream is;
		HuffmanBlk blk;
		int curblk;
		const HuffmanArray * data;
		friend class HuffmanArray;
	};
	void getEnum(unsigned int pos, Enum * e) const;
	typedef HuffmanArrBuilder BuilderTp;
private:
	mutable HuffmanBlk blk;
	mutable int curblk;

	unsigned int len, rate1, rate2;
	SDArraySml ptr;
	BitArray bits;
	
	friend class HuffmanArrBuilder;
	friend class Enum;
};

}//namespace
