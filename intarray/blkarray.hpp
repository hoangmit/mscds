#pragma once

#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>

#include "sdarray_sml.h"
#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

namespace mscds {

template<typename Model>
class CodeModelBlk {
public:
	void clear();
	void mload(const BitArray * enc, const SDArraySml * ptr, unsigned startpos);
	void build(std::vector<uint32_t> * data, unsigned int subsize, 
		OBitStream * out, std::vector<uint32_t> * opos);
	uint32_t lookup(unsigned int i) const;

	const Model& getModel() const;
private:
	Model model;
	const BitArray * enc;
	const SDArraySml * ptr;
	unsigned int subsize, len, blkpos;
};


template<typename Model>
class CodeModelArray;

template<typename Model>
class CodeModelBuilder {
public:
	typedef CodeModelArray<Model> QueryTp;

	CodeModelBuilder();
	void init(unsigned int rate = 64, unsigned int secondrate=512);
	void add(uint32_t val);
	void build(OArchive& ar);
	void build(QueryTp * out);
	void clear();
private:
	unsigned int rate1, rate2, cnt;
	CodeModelBlk<Model> blk;
	OBitStream out;
	std::vector<uint32_t> opos;
	std::vector<uint32_t> buf;
	SDArraySmlBuilder bd;
};

template<typename Model>
class CodeModelArray {
public:
	CodeModelArray(): curblk(-1) {}
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
		CodeModelBlk<Model> blk;
		int curblk;
		const CodeModelArray<Model> * data;
		friend class CodeModelArray<Model>;
	};
	void getEnum(unsigned int pos, Enum * e) const;
	typedef CodeModelBuilder<Model> BuilderTp;
private:
	mutable CodeModelBlk<Model> blk;
	mutable int curblk;

	unsigned int len, rate1, rate2;
	SDArraySml ptr;
	BitArray bits;

	friend class CodeModelBuilder<Model>;
	friend class Enum;
};

}//namespace

//------------------------------------------------------------------------------

namespace mscds {

template<typename Model>
void CodeModelBuilder<Model>::build(CodeModelArray<Model> * outds) {
	if (buf.size() > 0) {
		blk.build(&buf, rate1, &out, &opos);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
	out.close();
	bd.build(&(outds->ptr));
	outds->bits = BitArray::create(out.data_ptr(), out.length());
	outds->len = cnt;
	outds->rate1 = rate1;
	outds->rate2 = rate2;
}

template<typename Model>
void CodeModelBuilder<Model>::build(OArchive& ar ) {
	CodeModelArray<Model> tmp;
	tmp.save(ar);
}

template<typename Model>
void CodeModelBuilder<Model>::add(uint32_t val) {
	assert(rate1 > 0 && rate2 > 0);
	buf.push_back(val);
	cnt++;
	if (buf.size() % (rate1 * rate2) == 0) {
		blk.build(&buf, rate1, &out, &opos);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
}

template<typename Model>
void CodeModelBuilder<Model>::clear() {
	buf.clear(); bd.clear(); rate1 = 0; rate2 = 0; opos.clear();
}

template<typename Model>
void CodeModelBuilder<Model>::init(unsigned int rate /*= 64*/, unsigned int secondrate/*=512*/) {
	rate1 = rate; rate2 = secondrate;
	cnt = 0;
}

template<typename Model>
CodeModelBuilder<Model>::CodeModelBuilder() {
	init(64, 511);
}

template<typename Model>
void CodeModelBlk<Model>::mload(const BitArray * enc, const SDArraySml * ptr, unsigned pos) {
	auto st = ptr->prefixsum(pos);
	IWBitStream is(enc->data_ptr(), enc->length(), st);
	model.loadModel(is);
	subsize = is.get(32);
	len = is.get(32);
	this->enc = enc;
	this->ptr = ptr;
	this->blkpos = pos;
}

template<typename Model>
void CodeModelBlk<Model>::build(std::vector<uint32_t> * data, unsigned int subsize, OBitStream * out, std::vector<uint32_t> * opos) {
	clear();
	model.buildModel(data);
	auto cpos = out->length();
	model.saveModel(out);
	this->subsize = subsize;
	out->puts(subsize, 32);
	out->puts(data->size(), 32);

	for (int i = 0; i < data->size(); ++i) {
		if (i % subsize == 0) {
			opos->push_back(out->length() - cpos);
			cpos = out->length();
		}
		uint32_t val = (*data)[i];
		model.encode(val, out);
	}
	if (cpos != out->length())
		opos->push_back(out->length() - cpos);
}

template<typename Model>
uint32_t CodeModelBlk<Model>::lookup(unsigned int i) const {
	assert(i < len);
	auto r = i % subsize;
	auto p = i / subsize;
	auto epos = ptr->prefixsum(p + 1 + blkpos);
	IWBitStream is(enc->data_ptr(), enc->length(), epos);

	unsigned int val = 0;
	for (unsigned int j = 0; j <= r; ++j) {
		val = model.decode(&is);
	}
	return val;
}

template<typename Model>
uint32_t CodeModelArray<Model>::lookup(unsigned int i) const {
	assert(i < len);
	auto r = i % (rate1 * rate2);
	auto b = i / (rate1 * rate2);
	if (curblk != b) {
		auto blkst = (i / rate1)  - ((i / rate1) % rate2) + b;
		blk.mload(&bits, &ptr, blkst);
		curblk = b;
	}
	return blk.lookup(r);
}

template<typename Model>
void CodeModelArray<Model>::getEnum(unsigned int pos, Enum * e) const {
	assert(pos < len);
	e->pos = pos;
	auto r = pos % (rate1 * rate2);
	auto b = pos / (rate1 * rate2);
	auto blkst = (pos / rate1)  - ((pos / rate1) % rate2) + b;
	blk.mload(&bits, &ptr, blkst);
	auto i = r;
	r = i % rate1;
	auto p = i / rate1;
	auto epos = ptr.prefixsum(p + 1 + blkst);
	e->is.init(bits.data_ptr(), bits.length(), epos);
	e->data = this;
	for (unsigned int j = 0; j < r; ++j)
		blk.getModel().decode(&(e->is));
}

template<typename Model>
uint64_t CodeModelArray<Model>::Enum::next() {
	auto val = blk.getModel().decode(&is);
	++pos;
	if (pos < data->len) {
		auto bs = data->rate1 * data->rate2;
		if (pos % bs == 0) {
			auto b = pos / bs;
			auto blkst = pos/data->rate1 + b;
			blk.mload(&(data->bits), &(data->ptr), blkst);
			auto epos = data->ptr.prefixsum(1 + blkst);
			is.init(data->bits.data_ptr(), data->bits.length(), epos);
		}
	}
	return val;
}

template<typename Model>
void CodeModelBlk<Model>::clear() {
	enc = NULL;
	ptr = NULL;
	subsize = 0;
	len = 0;
	blkpos = 0;
	model.clear();
}

template<typename Model>
const Model& CodeModelBlk<Model>::getModel() const {
	return model;
}

template<typename Model>
void CodeModelArray<Model>::save(OArchive& ar) const {
	ar.startclass("huffman_code", 1);
	ar.var("length").save(len);
	ar.var("sample_rate").save(rate1);
	ar.var("bigblock_rate").save(rate2);
	ptr.save(ar.var("pointers"));
	bits.save(ar.var("bits"));
	ar.endclass();
}

template<typename Model>
void CodeModelArray<Model>::load(IArchive& ar) {
	ar.loadclass("huffman_code");
	ar.var("length").load(len);
	ar.var("sample_rate").load(rate1);
	ar.var("bigblock_rate").load(rate2);
	ptr.load(ar.var("pointers"));
	bits.load(ar.var("bits"));
	ar.endclass();
	curblk = -1;
}

template<typename Model>
void CodeModelArray<Model>::clear() {
	curblk = -1;
	bits.clear();
	ptr.clear();
	len = 0;
	rate1 = 0;
	rate2 = 0;
}

template<typename Model>
uint64_t CodeModelArray<Model>::length() const {
	return len;
}

} // namespace
