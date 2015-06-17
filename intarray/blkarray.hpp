#pragma once

/** \file

Array of integers that uses some generic compression method.

  The structure has 2 layers: big-block, and small-block. Compression method can have
  decoding information at the head of each big-block. Compression method should be
  able to randomly access to each small block.

  PointerModel: pointer calculation functions for 2-layer block structure

  CodeModelBlk<Model>: handle big-block.

  CodeModelBlk<Model>, CodeModelBuilder<Model>: the array 

*/

#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <memory>

#include "sdarray_sml.h"
#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"
#include "utils/utils.h"
//#include "utils/mm/cache_map.hpp"

namespace mscds {


template<typename Model>
class CodeModelArray;

/* two layers pointer model */
struct PointerModel {
	inline unsigned int p0(unsigned int pos) const { return pos / (rate1 * rate2); }
	inline unsigned int p1(unsigned int pos) const { return (pos % (rate1 * rate2)) / rate1; }
	inline unsigned int p3(unsigned int pos) const { return pos % rate1; }

	unsigned int blkptr(unsigned int blk) const { return ptr.prefixsum(blk * (rate2 + 1)); }
	unsigned int subblkptr(unsigned int blk, unsigned int subblk) const { return ptr.prefixsum(subblk + blk*(rate2 + 1) + 1); }
	unsigned int subblkptr(unsigned int pos) const { return ptr.prefixsum(pos / rate1 + p0(pos) + 1); }
	const BitArray& data() const { return bits; }
	void clear() { bits.clear(); len = 0; rate1 = 0; rate2 = 0; }
	unsigned int rate1, rate2;
	unsigned int len;
	SDArraySml ptr;
	BitArray bits;
};

template<typename Model>
class CodeModelBlk {
public:
	void clear();
	void mload(const PointerModel * ptr, unsigned int blk);
	void build(const std::vector<uint32_t> * data, unsigned int subsize, 
		OBitStream * out, std::vector<uint32_t> * opos, const Config* conf);

	void set_stream(unsigned int pos, IWBitStream& is) const;
	const Model& getModel() const;
	void inspect(const std::string& cmd, std::ostream& out) const;
	unsigned int get_blkid() const { return blk; }
private:
	Model model;
	const PointerModel * ptr;
	unsigned int blk, len;
};


template<typename Model>
class CodeModelBuilder {
public:
	typedef CodeModelArray<Model> QueryTp;

	CodeModelBuilder();
	~CodeModelBuilder();
	void init(const Config* conf = NULL);
	void add(uint32_t val);
	void build(OutArchive& ar);
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

/// Encoded array that requires seperated space for storing compression model
template<typename Model>
class CodeModelArray {
public:
	typedef std::shared_ptr<const CodeModelBlk<Model> > BlkPtr; 

	CodeModelArray() {}
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	uint64_t length() const;
	uint32_t lookup(unsigned int i) const;
	uint32_t operator[](unsigned int pos) const { return lookup(pos); }

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		bool hasNext() const { return pos < data->ptr.len;}
		uint64_t next();
	private:
		unsigned int pos;
		IWBitStream is;
		BlkPtr blk;
		const CodeModelArray<Model> * data;
		friend class CodeModelArray<Model>;
	};
	void getEnum(unsigned int pos, Enum * e) const;
	typedef CodeModelBuilder<Model> BuilderTp;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	mutable std::unordered_map<unsigned int, BlkPtr > blkcache;

	BlkPtr getBlk(unsigned int blk) const;

	PointerModel ptr;

	friend class CodeModelBuilder<Model>;
	friend struct Enum;
};


}//namespace

//------------------------------------------------------------------------------

namespace mscds {


template<typename Model>
void CodeModelBlk<Model>::build(const std::vector<uint32_t> * data, unsigned int subsize,
	OBitStream * out, std::vector<uint32_t> * opos, const Config* conf) {
	clear();
	model.buildModel(data, conf);
	auto cpos = out->length();
	model.saveModel(out);
	//this->subsize = subsize;
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
void CodeModelBlk<Model>::mload(const PointerModel * ptr, unsigned int blk) {
	this->ptr = ptr;
	IWBitStream is;
	is.init(&(ptr->bits), ptr->blkptr(blk));
	model.loadModel(is);
	is.get(32); // unsigned int subsize =  // avoid unused variable warning
	this->len = is.get(32);
	this->blk = blk;
}

template<typename Model>
void CodeModelBlk<Model>::set_stream(unsigned int i, IWBitStream& is) const {
	assert(ptr->p0(i) == this->blk);
	is.init(&(ptr->bits), ptr->subblkptr(blk, ptr->p1(i)));
	auto r = ptr->p3(i);

	for (unsigned int j = 0; j < r; ++j)
		model.decode(&is);
}


template<typename Model>
void mscds::CodeModelBlk<Model>::inspect(const std::string& cmd, std::ostream& out) const {
	model.inspect(cmd, out);
}

//------------------------------------------------

template<typename Model>
void CodeModelBuilder<Model>::build(OutArchive& ar) {
	CodeModelArray<Model> tmp;
	tmp.save(ar);
}

template<typename Model>
void CodeModelBuilder<Model>::build(CodeModelArray<Model> * outx) {
	if (buf.size() > 0) {
		blk.build(&buf, rate1, &out, &opos, NULL);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
	out.close();
	auto& outds = outx->ptr;
	bd.build(&(outds.ptr));
	out.build(&outds.bits);
	outds.len = cnt;
	outds.rate1 = rate1;
	outds.rate2 = rate2;
}

template<typename Model>
void CodeModelBuilder<Model>::add(uint32_t val) {
	assert(rate1 > 0 && rate2 > 0);
	buf.push_back(val);
	cnt++;
	if (buf.size() % (rate1 * rate2) == 0) {
		blk.build(&buf, rate1, &out, &opos, NULL);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
}

template<typename Model>
void CodeModelBuilder<Model>::clear() {
	out.clear();
	buf.clear(); bd.clear(); rate1 = 0; rate2 = 0; opos.clear();
}

template<typename Model>
void CodeModelBuilder<Model>::init(const Config* conf /*= NULL */) {
	if (conf == NULL) {
		rate1 = 64; rate2 = 2047;
	} else {
		rate1 = conf->getInt("SAMPLE_RATE", 64);
		rate2 = conf->getInt("BLOCK_RATE", 2047);
	}
	cnt = 0;
}

template<typename Model>
CodeModelBuilder<Model>::CodeModelBuilder() {
	init();
}

template<typename Model>
CodeModelBuilder<Model>::~CodeModelBuilder() 
{ out.close(); }

template<typename Model>
typename CodeModelArray<Model>::BlkPtr CodeModelArray<Model>::getBlk(unsigned int b) const {
	auto it = blkcache.find(b);
	if (it != blkcache.end()) {
		return it->second;
	}else {
		CodeModelBlk<Model> * bx = new CodeModelBlk<Model>();
		bx->mload(&ptr, b);
		BlkPtr blk = BlkPtr(bx);
		blkcache.insert(make_pair(b, blk));
		return blk;
	}
}

template<typename Model>
uint32_t CodeModelArray<Model>::lookup(unsigned int i) const {
	Enum e;
	getEnum(i, &e);
	return e.next();
}

template<typename Model>
void CodeModelArray<Model>::getEnum(unsigned int pos, Enum * e) const {
	e->data = this;
	e->pos = pos;
	e->blk = getBlk(ptr.p0(pos));
	e->blk->set_stream(pos, e->is);
}

template<typename Model>
uint64_t CodeModelArray<Model>::Enum::next() {
	auto val = blk->getModel().decode(&is);
	++pos;
	if (pos < data->ptr.len) {
		auto bs = data->ptr.rate1 * data->ptr.rate2;
		if (pos % bs == 0) {
			blk = data->getBlk(pos / bs);
			blk->set_stream(pos, is);
		}
	}
	return val;
}

template<typename Model>
void mscds::CodeModelArray<Model>::inspect(const std::string& cmd, std::ostream& out) const {
	unsigned int i = 0, p = 0;
	while (i < ptr.len) {
		getBlk(p)->getModel().inspect(cmd, out);
		i += ptr.rate1 * ptr.rate2;
		p += 1;
	}
}

template<typename Model>
void CodeModelBlk<Model>::clear() {
	ptr = NULL;
	len = 0;
	model.clear();
}

template<typename Model>
const Model& CodeModelBlk<Model>::getModel() const {
	return model;
}

template<typename Model>
void CodeModelArray<Model>::save(OutArchive& ar) const {
	ar.startclass(std::string("code_") + TypeParseTraits<Model>::name(), 1);
	ar.var("length").save(ptr.len);
	ar.var("sample_rate").save(ptr.rate1);
	ar.var("bigblock_rate").save(ptr.rate2);
	ptr.ptr.save(ar.var("pointers"));
	ptr.bits.save(ar.var("bits"));
	ar.endclass();
}

template<typename Model>
void CodeModelArray<Model>::load(InpArchive& ar) {
	ar.loadclass(std::string("code_") + TypeParseTraits<Model>::name());
	ar.var("length").load(ptr.len);
	ar.var("sample_rate").load(ptr.rate1);
	ar.var("bigblock_rate").load(ptr.rate2);
	ptr.ptr.load(ar.var("pointers"));
	ptr.bits.load(ar.var("bits"));
	ar.endclass();
	blkcache.clear();
}

template<typename Model>
void CodeModelArray<Model>::clear() {
	ptr.clear();
	blkcache.clear();
}

template<typename Model>
uint64_t CodeModelArray<Model>::length() const {
	return ptr.len;
}

} // namespace
