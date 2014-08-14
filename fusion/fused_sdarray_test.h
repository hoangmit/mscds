
#pragma once

#include "block_mem_mng.h"
#include "sdarray_block.h"

struct MockBlk {
	const static uint64_t header = 0x0102030405060708ull;
	uint16_t v;

	void saveBlock(mscds::OBitStream * bs) {
		uint64_t v = header;
		bs->puts(v);
		v = this->v;
		bs->puts(v, 16);
	}

	void loadBlock(const mscds::BitRange& br) {
		loadBlock(*br.ba, br.start, br.len);
	}

	void loadBlock(const mscds::BitArray& ba, size_t pt, size_t len) {
		uint64_t x;
		assert(64 + 16 <= len);
		x = ba.bits(pt, 64);
		if (x != header) {
			throw std::runtime_error("wrong load");
		}

		v = ba.bits(pt + 64, 16);
	}
};

struct MockInterBlkQr: public mscds::InterBLockQueryTp {
	unsigned int sid, did;
	MockInterBlkQr(): mng(nullptr), sid(0), did(0) {}
	MockBlk blk;
	mscds::BlockMemManager * mng;

	void setup(mscds::BlockMemManager& _mng, mscds::StructIDList& lst) {
		mng = &_mng;
		lst.checkId("mock");
		sid = lst.get();
		did = lst.get();
		assert(sid > 0);
		assert(did > 0);
	}

	void check(unsigned int blkid) {
		assert(1 == mng->getGlobal(sid).byte(0));
		uint64_t x = mng->getSummary(sid, blkid).byte(0);
		x |= mng->getSummary(sid, blkid).byte(1) << 8;
		assert(blkid % 1000 == x);
		blk.loadBlock(mng->getData(did, blkid));
		assert(blkid == blk.v);
	}

	void inspect(const std::string &cmd, std::ostream &out) {}
	void clear() { mng = nullptr; sid = 0; did = 0; }
};

struct MockInterBlkBd: public mscds::InterBlockBuilderTp {
	unsigned int cnt;
	MockInterBlkBd(mscds::BlockBuilder& bd_): bd(&bd_) {}
	MockInterBlkBd(): bd(nullptr) {}

	void init_bd(mscds::BlockBuilder& bd_) { bd = &bd_; }

	void register_struct() {
		sid = bd->register_summary(1, 2);
		did = bd->register_data_block();
		cnt = 0;
	}

	bool is_empty() const { return false; }

	bool is_full() const { return false; }

	void set_block_data(bool lastblock = false) {
		uint16_t tt;
		tt = cnt % 1000;
		bd->set_summary(sid, mscds::MemRange::wrap(tt));
		mscds::OBitStream& d1 = bd->start_data(did);
		blk.v = cnt;
		blk.saveBlock(&d1);
		bd->end_data();
		cnt++;
	}

	void build_struct() {
		uint8_t v = 1;
		bd->set_global(sid, mscds::MemRange::wrap(v));
	}

	void deploy(mscds::StructIDList& lst) {
		lst.addId("mock");
		lst.add(sid);
		lst.add(did);
	}

	void clear() { sid = 0; did = 0; }

	MockBlk blk;
	unsigned int sid, did;
	mscds::BlockBuilder * bd;
};

struct TwoSDA_Query {
	mscds::BlockMemManager mng;
	void clear() { mng.clear(); mock.clear(); }
	MockInterBlkQr mock;
	mscds::SDArrayFuse x, y;

	void init(mscds::StructIDList& lst) {
		mock.setup(mng, lst);
		x.setup(mng, lst);
		y.setup(mng, lst);
	}
};

struct TwoSDA_Builder {
	mscds::BlockBuilder bd;
	MockInterBlkBd mbx;
	mscds::SDArrayFuseBuilder bd1, bd2;
	unsigned int cnt;
	TwoSDA_Builder(): bd1(bd), bd2(bd), mbx(bd) {}

	void init() {
		mbx.register_struct();
		bd1.register_struct();
		bd2.register_struct();
		cnt = 0;
		bd.init_data();
		blkcntx = 0;
	}

	void add(unsigned int x, unsigned int y) {
		bd1.add(x);
		bd2.add(y);
		cnt++;
		if (cnt == 512) {
			_end_block();
			cnt = 0;
			blkcntx++;
		}
	}

	void _end_block() {
		mbx.set_block_data();
		bd1.set_block_data();
		bd2.set_block_data();
		bd.end_block();
	}

	void build(TwoSDA_Query * out) {
		_end_block();
		mbx.build_struct();
		bd1.build_struct();
		bd2.build_struct();
		bd.build(&out->mng);
		mscds::StructIDList lst;
		mbx.deploy(lst);
		bd1.deploy(lst);
		bd2.deploy(lst);
		out->init(lst);
	}
	unsigned int blkcntx;
};

