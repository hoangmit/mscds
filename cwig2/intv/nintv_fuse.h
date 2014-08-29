#pragma once

#include "cwig/intv/nintv.h"
#include "fusion/block_mem_mng.h"
#include "fusion/sdarray_block.h"
#include "fusion/generic_struct.h"
#include "fusion/sdarray_blk_hints.h"

#include <algorithm>

namespace app_ds {

class NIntvInterBlkBuilder;

class FuseNIntvInterBlock: public NIntvQueryInt, public mscds::InterBlockQueryTp {
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
	void clear();
	void inspect(const std::string &cmd, std::ostream &out) {}
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
	mscds::SDArrayFuseHints start;
	mscds::BlockMemManager * mng;
};


class NIntvInterBlkBuilder: public mscds::InterBlockBuilderTp {
public:
	NIntvInterBlkBuilder(mscds::BlockBuilder& _bd): bd(&_bd), cnt(0) {}
	NIntvInterBlkBuilder(): bd(nullptr) {}

	void init_bd(mscds::BlockBuilder& bd_) { bd = &bd_; start.init_bd(bd_); }

	void start_model() { start.start_model(); added = false; }
	void model_add(unsigned int st) { if (added) { start.model_add(st - last); } else added = true; last = st; }
	void build_model() { start.build_model(); }

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
	mscds::SDArrayFuseHintsBuilder start;
	unsigned int cnt;

	std::vector<std::pair<unsigned int, unsigned int> > data;

private:
	bool added;
	unsigned int last;
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
