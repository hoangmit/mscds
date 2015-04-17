#include "sdarray_block.h"

namespace mscds {

SDArrayFuseBuilder::SDArrayFuseBuilder(BlockBuilder &_bd) : bd(&_bd) {}

SDArrayFuseBuilder::SDArrayFuseBuilder(): bd(nullptr) {}

void SDArrayFuseBuilder::init_bd(BlockBuilder &bd_) { bd = &bd_; }

void SDArrayFuseBuilder::register_struct() {
	bd->begin_scope("sdarray");
	sid = bd->register_summary(16, 8); // bytes
	did = bd->register_data_block();
	bd->end_scope();
	cnt = 0;
	sum = 0;
	blkcnt = 0;
	lastsum = 0;
}

void SDArrayFuseBuilder::add(uint64_t val) {
	assert(!is_full());
	blk.add(val);
	sum += val;
	cnt++;
	blkcnt++;
	assert(blkcnt <= blk.BLKSIZE);
}

bool SDArrayFuseBuilder::is_empty() const { return blkcnt == 0; }

bool SDArrayFuseBuilder::is_full() const { return blkcnt >= blk.BLKSIZE; }

void SDArrayFuseBuilder::set_block_data(bool lastblock) {
	if (blkcnt > 0) {
		uint64_t v = lastsum;
		bd->set_summary(sid, ByteMemRange::ref(v));
		OBitStream& d1 = bd->start_data(did);
		blk.saveBlock(&d1);
		bd->end_data();
		blkcnt = 0;
		lastsum = sum;
	}
}

void SDArrayFuseBuilder::build_struct() {
	assert(is_empty());
	struct {
		uint64_t cnt, sum;
	} data;
	data.cnt = cnt;
	data.sum = sum;
	bd->set_global(sid, ByteMemRange::ref(data));
}

void SDArrayFuseBuilder::deploy(StructIDList& lst) {
	lst.addId("sdarray");
	lst.add(sid);
	lst.add(did);
}

SDArrayFuse::ValueType SDArrayFuse::prefixsum(unsigned int p) const {
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

SDArrayFuse::ValueType SDArrayFuse::lookup(unsigned int p) const {
	uint64_t bpos = p / SDArrayBlock::BLKSIZE;
	uint32_t off = p % SDArrayBlock::BLKSIZE;
	loadBlk(bpos);
	return blk.lookup(off);
}

SDArrayFuse::ValueType SDArrayFuse::lookup(unsigned int p, SDArrayFuse::ValueType &prev_sum) const {
	uint64_t bpos = p / SDArrayBlock::BLKSIZE;
	uint32_t off = p % SDArrayBlock::BLKSIZE;
	auto sum = getBlkSum(bpos);
	loadBlk(bpos);
	auto v = blk.lookup(p, prev_sum);
	prev_sum += sum;
	return v;
}

unsigned int SDArrayFuse::rank(SDArrayFuse::ValueType val) const {
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
	return ret;
}

unsigned int SDArrayFuse::_rank(SDArrayFuse::ValueType val, unsigned int begin, unsigned int end) const {
	if (val > total_sum()) return length();
	uint64_t lo = begin;
	uint64_t hi = end;
	assert(lo <= hi && hi <= mng->blkCount());
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
	return ret;
}

void SDArrayFuse::setup(BlockMemManager &mng_, StructIDList &lst) {
	mng = &mng_;
	lst.checkId("sdarray");
	sid = lst.get();
	did = lst.get();
	assert(sid > 0);
	assert(did > 0);
	load_global();
}

SDArrayFuse::SDArrayFuse(BlockMemManager &mng_, unsigned sid_, unsigned did_):
	mng(&mng_), sid(sid_), did(did_) { load_global(); }

uint64_t SDArrayFuse::getBlkSum(size_t blk) const {
	//auto br = mng->getSummary(sid, blk);
	//return br.bits(0, 64);
	return mng->summary_word(sid, blk);
}

void SDArrayFuse::clear() {
	mng = nullptr;
	len = 0;
	sum = 0;
	sid = 0;
	did = 0;
	blk.clear();
}

void SDArrayFuse::inspect(const std::string &cmd, std::ostream &out) {}

void SDArrayFuse::load_global() {
	auto br = mng->getGlobal(sid);
	len = br.bits(0, 64);
	sum = br.bits(64, 64);
}

void SDArrayFuse::loadBlk(size_t i) const {
	auto br = mng->getData(did, i);
	blk.loadBlock(br);
}

}//namespace

