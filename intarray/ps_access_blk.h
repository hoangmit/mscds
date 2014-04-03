
#pragma once

#include "blkgroup_array.h"
#include "inc_ptrs.h"

namespace mscds {

class PtrInterBlkBd: public InterBlockBuilderTp {
public:
	PtrInterBlkBd(): size(0) {}
	void init_bd(BlockBuilder& bd_) { bd = &bd_; }

	void register_struct() {
		assert(size  > 0);
		sid = bd->register_summary(0,8);
		did = bd->register_data_block();
		lastval = 0;
		firstblkitem = 0;
		cnt = 0;
	}

	void init_blk(unsigned int size_) {
		size = size_;
		blk.init(size_);
	}

	void add(uint64_t val) {
		assert(val >= lastval);
		uint64_t diff = val - lastval;
		blk.add(diff);
		val = lastval;
		cnt++;
		if (cnt % size == 0) {
			firstblkitem = val;
		}
	}

	void set_block_data(bool lastblock = false) {
		while (cnt % size != 0) {
			blk.add(0);
			cnt++;
		}
		blk._build();
		bd->set_summary(sid, MemRange::wrap(firstblkitem));
		auto& a = bd->start_data(did);
		blk.saveBlock(&a);
		bd->end_data();
	}

	void build_struct() {
		bd->set_global(sid);
	}

	void deploy(StructIDList & lst) {
		lst.addId("ptr_raw");
		lst.add(sid);
		lst.add(did);
		lst.add(size);
	}
private:
	uint64_t lastval, firstblkitem;
	BlockBuilder * bd;
	FixBlockPtr blk;
	unsigned int sid, did;
	unsigned int size, cnt;
};

class PtrInterBlkQs: public InterBLockQueryTp {
public:
	PtrInterBlkQs(): size(0) {}

	void setup(BlockMemManager & mng_, StructIDList& slst) {
		slst.checkId("ptr_raw");
		sid = slst.get();
		did = slst.get();
		size = slst.get();
		mng = &mng_;
		lastblk = ~0u;
		blk.init(size);
		assert(size > 0);
	}
	uint64_t get(unsigned int p) {
		load_block(p % size);
		return blk.start(p) + fv;
	}
private:
	void load_block(unsigned int blkid) {
		if (lastblk == blkid) return ;
		fv = mng->getSummary(sid, blkid).bits(0, 64);
		auto br = mng->getData(did, blkid);
		blk.loadBlock(*br.ba, br.start, br.len);
		lastblk = blkid;
	}

	BlockMemManager * mng;

	unsigned int lastblk;
	uint64_t fv;
	FixBlockPtr blk;
	unsigned int sid, did;
	unsigned int size;
};

}//namespace