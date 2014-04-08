#pragma once

#include <vector>
#include <stdint.h>
#include "bitarray/bitstream.h"
#include "bitarray/bitarray.h"
#include "blkgroup_array.h"

namespace mscds {

//template<typename BlockManager>
class SDArrayFuse;

class SDArrayBlock {
public:
	typedef uint64_t ValueType;
	typedef unsigned int IndexType;
	void add(ValueType v);
	void saveBlock(OBitStream* bits);
	void loadBlock(const BitRange& br);
	void loadBlock(const BitArray & ba, size_t pt, size_t len);

	ValueType prefixsum(unsigned int  p) const;
	ValueType lookup(unsigned int p) const;
	ValueType lookup(unsigned int p, ValueType& prev_sum) const;
	unsigned int rank(ValueType val) const;

	void clear() { lastpt = ~0ULL; vals.clear(); bits.clear(); width = 0; select_hints = 0; blkptr = 0; }
	SDArrayBlock() { clear(); }
	static const unsigned int BLKSIZE = 512;
private:
	unsigned int select_hi(uint64_t hints, uint64_t start, uint32_t off) const;
	unsigned int scan_hi_bits(uint64_t start, uint32_t res) const;
	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
	unsigned int scan_hi_next(unsigned int start) const;
	unsigned int select_zerohi(uint64_t hints, uint64_t start, uint32_t off) const;
	unsigned int scan_hi_zeros(unsigned int start, uint32_t res) const;
private:
	//static const unsigned int BLKSIZE = 512;
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;

	std::vector<ValueType> vals;

	uint16_t width;
	uint64_t select_hints;

	size_t lastpt;

	size_t blkptr;
	BitArray bits;
	//template<typename>
	friend class SDArrayFuse;
};

class SDArrayFuse;

class SDArrayFuseBuilder: public InterBlockBuilderTp {
public:
	SDArrayFuseBuilder(BlockBuilder& _bd) : bd(&_bd) {}
	SDArrayFuseBuilder(): bd(nullptr) {}

	void init_bd(BlockBuilder& bd_) { bd = &bd_; }
	void register_struct();

	void add(uint64_t val);
	void add_incnum(uint64_t val);
	bool is_empty() const;
	bool is_full() const;
	void set_block_data(bool lastblock = false);
	void build_struct();
	void deploy(StructIDList& lst);

	unsigned int blk_size() const { return 512; }
private:
	uint64_t last;
	SDArrayBlock blk;
	unsigned int sid, did;
	BlockBuilder * bd;
	uint64_t sum, cnt, lastsum;
	int blkcnt;
};

//template<typename BlockManager>
class SDArrayFuse : public InterBLockQueryTp {
public:
	typedef SDArrayBlock BlockType;
	typedef SDArrayBlock::ValueType ValueType;

	ValueType prefixsum(unsigned int  p) const {
		if (p == this->len) return this->sum;
		uint64_t bpos = p / SDArrayBlock::BLKSIZE;
		uint32_t off = p % SDArrayBlock::BLKSIZE;
		auto sum = getBlkSum(bpos);
		if (off == 0) return sum;
		else {
			loadBlk(bpos);
			return sum + blk.prefixsum(off);
		}
	}

	ValueType lookup(unsigned int p) const {
		uint64_t bpos = p / SDArrayBlock::BLKSIZE;
		uint32_t off = p % SDArrayBlock::BLKSIZE;
		loadBlk(bpos);
		return blk.lookup(off);
	}

	ValueType lookup(unsigned int p, ValueType& prev_sum) const {
		uint64_t bpos = p / SDArrayBlock::BLKSIZE;
		uint32_t off = p % SDArrayBlock::BLKSIZE;
		auto sum = getBlkSum(bpos);
		loadBlk(bpos);
		auto v = blk.lookup(p, prev_sum);
		prev_sum += sum;
		return v;
	}

	unsigned int rank(ValueType val) const {
		if (val > total_sum()) return length();
		uint64_t lo = 0;
		uint64_t hi = mng->blkCount();
		while (lo < hi) {
			uint64_t mid = lo + (hi - lo) / 2;
			if (getBlkSum(mid) < val) lo = mid + 1;
			else hi = mid;
		}
		if (lo == 0) return 0;
		lo--;
		assert(val > getBlkSum(lo));
		assert(lo < mng->blkCount() || val <= getBlkSum(lo + 1));
		loadBlk(lo);
		ValueType ret = lo * SDArrayBlock::BLKSIZE + blk.rank(val - getBlkSum(lo));
		if (ret == 0) return 0;
		else return ret - 1;
	}
	SDArrayFuse() : mng(nullptr), len(0) {}
	void setup(BlockMemManager& mng_, StructIDList& lst) {
		mng = &mng_;
		lst.checkId("sdarray");
		sid = lst.get();
		did = lst.get();
		assert(sid > 0);
		assert(did > 0);
		load_global();
	}
	SDArrayFuse(BlockMemManager& mng_, unsigned sid_, unsigned did_):
		mng(&mng_), sid(sid_), did(did_) { load_global(); }
	uint64_t length() const { return len; }

	uint64_t getBlkSum(size_t blk) const {
		auto br = mng->getSummary(sid, blk);
		return br.bits(0, 64);
	}

	void clear() {
		mng = nullptr;
		len = 0;
		sum = 0;
		sid = 0;
		did = 0;
		blk.clear();
	}

	void inspect(const std::string &cmd, std::ostream &out) {}

private:
	friend class SDArrayFuseBuilder;

	unsigned int sid, did;
	BlockMemManager* mng;
	uint64_t len, sum;

	void load_global() {
		auto br = mng->getGlobal(sid);
		len = br.bits(0, 64);
		sum = br.bits(64, 64);
	}

	uint64_t total_sum() const { return sum; }

	void loadBlk(size_t i) const {
		auto br = mng->getData(did, i);
		blk.loadBlock(br);
	}

	mutable SDArrayBlock blk;
};

}
