
#pragma once

/** \file

Experimental fusion related structure.
*/

#include "generic_struct.h"
#include "block_mem_mng.h"
#include "inc_ptrs.h"
#include "codec/deltacoder.h"

namespace mscds {

class PtrInterBlkBd: public InterBlockBuilderTp {
public:
	PtrInterBlkBd(): size(0) {}
	void init_bd(BlockBuilder& bd_) { bd = &bd_; }

	void init_blk(unsigned int size_) {
		size = size_;
		assert(size_ > 0);
		blk.init(size_ - 1);
	}

	void register_struct() {
		assert(size  > 0);
		bd->begin_scope("PtrInterBlk");
		sid = bd->register_summary(0,0);
		did = bd->register_data_block();
		bd->end_scope();
		lastval = 0;
		firstblkitem = 0;
		bcnt = 0;
	}

	void add(uint64_t val) {
		assert(val >= lastval);
		assert(!is_full());
		if (bcnt == 0) {
			firstblkitem = val;
			lastval = val;
		} else {
			uint64_t diff = val - lastval;
			blk.add(diff);
		}
		lastval = val;
		bcnt++;
	}

	bool is_empty() const { return bcnt == 0; }

	bool is_full() const { return bcnt >= size; }

	void set_block_data(bool lastblock = false) {
		while (bcnt < size) {
			blk.add(0);
			bcnt++;
		}
		blk._build();
		bd->set_summary(sid);
		auto& a = bd->start_data(did);
		auto cp = coder::DeltaCoder::encode(firstblkitem + 1);
		a.puts(cp);
		blk.saveBlock(&a);
		bd->end_data();
		blk.reset();
		bcnt = 0;
		firstblkitem = 0;
		lastval = 0;
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
	unsigned int size;
	unsigned int bcnt;
};

class PtrInterBlkQs: public InterBlockQueryTp {
public:
	PtrInterBlkQs(): size(0), mng(nullptr) {}

	void inspect(const std::string &cmd, std::ostream &out) {}

	void setup(BlockMemManager & mng_, StructIDList& slst) {
		slst.checkId("ptr_raw");
		sid = slst.get();
		did = slst.get();
		size = slst.get();
		mng = &mng_;
		lastblk = ~0u;
		assert(size > 0);
		blk.init(size - 1);
	}
	uint64_t get(unsigned int p) {
		auto bi = p / size;
		auto ip = p % size;
		load_block(bi);
		if (ip == 0) return fv;
		else
			return blk.start(ip) + fv;
	}

	void clear() {
		mng = nullptr;
		fv = 0;
		blk.clear();
		sid = did = 0;
		size = 0;
	}
private:
	void load_block(unsigned int blkid) {
		if (lastblk == blkid) return ;
		//fv = mng->getSummary(sid, blkid).bits(0, 64);
		auto br = mng->getData(did, blkid);
		uint64_t v = br.bits(0, std::min<unsigned>(br.len, 64));
		auto cp = coder::DeltaCoder::decode2(v);
		cp.first -= 1;
		fv = cp.first;
		blk.loadBlock(*br.ba, br.start + cp.second, br.len - cp.second);
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
