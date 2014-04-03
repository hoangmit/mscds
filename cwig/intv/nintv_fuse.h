#pragma once

#include "nintv.h"
#include "intarray/blkgroup_array.h"
#include "intarray/sdarray_block.h"

#include <algorithm>

namespace app_ds {

class NIntvInterBlkBuilder;

class FuseNIntvInterBlock: public NIntvQueryInt, public mscds::InterBLockQueryTp {
public:
	PosType int_start(PosType i) const;
	PosType int_end(PosType i) const;
	std::pair<PosType, PosType> int_startend(PosType i) const;

	PosType int_len(PosType i) const;
	std::pair<PosType, PosType> find_cover(PosType pos) const;
	PosType coverage(PosType pos) const;

	PosType rank_interval(PosType pos) const;
	PosType int_psrlen(PosType i) const;

	PosType length() const { return start.length(); }
	
	FuseNIntvInterBlock();

	static PosType npos() { return std::numeric_limits<PosType>::max(); }

	void setup(mscds::BlockMemManager& mng_, mscds::StructIDList& lst);
private:
	const static unsigned int BLKSIZE = 512;
	struct LGBlk_t {
		uint64_t sum;
		bool store_len;
	};
	LGBlk_t loadLGSum(unsigned int blk) const;

	void loadblock(unsigned int blk) const;
	void loadGlobal();

	friend class NIntvInterBlkBuilder;
	mutable mscds::SDArrayBlock lgblk;
	unsigned int glsid, gldid;
	uint64_t lensum;
	mscds::SDArrayFuse start;
	mscds::BlockMemManager * mng;
	
};

//void FuseNIntvInterBlock::loadblock(unsigned int blk) const;

class NIntvInterBlkBuilder: public mscds::InterBlockBuilderTp {
public:
	NIntvInterBlkBuilder(mscds::BlockBuilder& _bd): bd(&_bd), start(_bd), cnt(0) {}
	NIntvInterBlkBuilder(): bd(nullptr) {}

	void init_bd(mscds::BlockBuilder& bd_) { bd = &bd_; start.init_bd(bd_); }

	void register_struct();
	void add(unsigned int st, unsigned int ed);
	bool is_empty() const;
	bool is_full() const;
	void set_block_data(bool lastblock = false);
	void build_struct();
	void deploy(mscds::StructIDList& lst);
private:
	void _build_block();
	void _build_block_type(bool store_len);

	mscds::SDArrayBlock lgblk;
	unsigned int lgsid, lgdid;

	uint64_t lensum, laststart;

	mscds::BlockBuilder * bd;
	mscds::SDArrayFuseBuilder start;
	unsigned int cnt;

	std::vector<std::pair<unsigned int, unsigned int> > data;

private:
	mscds::BlockMemManager mng;
	void clear() { mng.clear(); }
};

class NIntvFuseBuilder;

class NIntvFuseQuery {
public:
	FuseNIntvInterBlock b;
	void init(mscds::StructIDList& lst);

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
private:
	mscds::BlockMemManager mng;
	friend class NIntvFuseBuilder;
};

class NIntvFuseBuilder {
public:
	NIntvFuseBuilder(): cnt(0), bd(), iblk(bd) {}

	void init();
	void add(unsigned int st, unsigned int ed);
	void _end_block();

	void build(NIntvFuseQuery* out);
private:
	size_t cnt;
	mscds::BlockBuilder bd;
	NIntvInterBlkBuilder iblk;
};



}//namespace
