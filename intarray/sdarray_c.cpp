#include "sdarray_c.h"


namespace mscds {

SDArrayCompressBuilder::SDArrayCompressBuilder() { init(); }

void SDArrayCompressBuilder::init() {
	i = 0; sum = 0;
	static_assert(BLKSIZE == SDArrayBlock2::BLKSIZE + 1, "mismatch BLKSIZE");
}

void SDArrayCompressBuilder::add(unsigned int v) {
	sum += v;
	++i;
	if (i % BLKSIZE == 0) {
		// skip the BLKSIZE-th value in the block
		//each block has (BLKSIZE-1) = 1023 values
		_build_blk();
	} else {
		blk.add(v);
	}
}

void SDArrayCompressBuilder::_build_blk() {
	blk.saveBlock(&obits);
	blkpos.push_back(obits.length());
	csum.push_back(sum);
}

void SDArrayCompressBuilder::_finalize() {
	if (i % BLKSIZE != 0) {
		blk.saveBlock(&obits);
	}
	if (i > 0) {
		blkpos.push_back(obits.length());
		csum.push_back(sum);
	}
	if (!blkpos.empty()) {
		w1 = val_bit_len(csum.back());
		w2 = val_bit_len(blkpos.back());
	} else {
		w1 = w2 = 0;
	}
	assert(csum.size() == blkpos.size());
	auto it = csum.begin();
	auto jt = blkpos.begin();
	while (it != csum.end()) {
		header.puts(*it,w1);
		header.puts(*jt,w2);
		++it;
		++jt;
	}
	w2 += w1;

	blkpos.clear();
	csum.clear();
}

void SDArrayCompressBuilder::add_inc(unsigned int s) {
	assert(s >= sum);
	add(s - sum);
}

void SDArrayCompressBuilder::build(SDArrayCompress *out) {
	_finalize();
	header.build(&out->header);
	BitArray ba;
	obits.build(&ba);
	RRR_BitArrayBuilder::build_array(ba, &(out->cbits));
	out->w1 = w1;
	out->w2 = w2;
	out->sum = sum;
	out->len = i;
}

void SDArrayCompressBuilder::build_aux(SDArrayBAux *out) {
	_finalize();
	header.build(&out->header);
	obits.build(&(out->own_bits));
	out->w1 = w1;
	out->w2 = w2;
	out->sum = sum;
	out->len = i;
}

SDArrayBAux::ValueTp SDArrayBAux::_getBlkSum(unsigned blk) const {
	if (blk == 0) return 0;
	else blk-=1;
	return header.bits(w2*blk, w1);
}

SDArrayBAux::ValueTp SDArrayBAux::_getBlkStartPos(unsigned blk) const {
	if (blk == 0) return 0;
	else blk-=1;
	return header.bits(w2*blk + w1, w2-w1);
}

void SDArrayBAux::_loadBlk(unsigned bp) const {
	auto p1 = _getBlkStartPos(bp);
	auto p2 = _getBlkStartPos(bp+1);
	blk.loadBlock(bits, p1, p2-p1);
}


SDArrayBAux::SDArrayBAux(): bits(nullptr), len(0), sum(0) {}

SDArrayBAux::ValueTp SDArrayBAux::prefixsum(ValueTp p) const {
	if (p == this->len) return this->sum;
	uint64_t bpos = p / BLKSIZE;
	uint32_t off = p % BLKSIZE;
	auto sum = _getBlkSum(bpos);
	if (off == 0) return sum;
	else {
		_loadBlk(bpos);
		return sum + blk.prefixsum(off);
	}
}

SDArrayBAux::ValueTp SDArrayBAux::lookup(ValueTp p) const {
	uint64_t bpos = p / BLKSIZE;
	uint32_t off = p % BLKSIZE;
	if (off + 1 < BLKSIZE) {
		_loadBlk(bpos);
		return blk.lookup(off);
	}else {
		auto sum0 = _getBlkSum(bpos);
		auto sum1 = _getBlkSum(bpos + 1);
		_loadBlk(bpos);
		return sum1 - sum0 - blk.prefixsum(BLKSIZE-1);
	}
}

SDArrayBAux::ValueTp SDArrayBAux::lookup(ValueTp p, SDArrayBAux::ValueTp &prev_sum) const {
	uint64_t bpos = p / BLKSIZE;
	uint32_t off = p % BLKSIZE;
	if (off + 1 < BLKSIZE) {
		_loadBlk(bpos);
		auto ret = blk.lookup(off, prev_sum);
		prev_sum += _getBlkSum(bpos);
		return ret;
	} else {
		auto sum0 = _getBlkSum(bpos);
		auto sum1 = _getBlkSum(bpos + 1);
		_loadBlk(bpos);
		auto vz = sum0 + blk.prefixsum(BLKSIZE-1);
		auto ret = sum1 - vz;
		prev_sum = vz;
		return ret;
	}
}

SDArrayBAux::ValueTp SDArrayBAux::rank(ValueTp val) const {
	if (val > total()) return length();
	size_t maxl = header.length() / w2;
	uint64_t lo = 0;
	uint64_t hi = maxl;
	while (lo < hi) {
		uint64_t mid = lo + (hi - lo) / 2;
		if (_getBlkSum(mid) < val) lo = mid + 1;
		else hi = mid;
	}
	if (lo == 0) return 0;
	else { 
	}
	lo--;
	auto lsum = _getBlkSum(lo);
	assert(val > lsum);
	assert(lo + 1 < maxl || val <= _getBlkSum(lo + 1));
	if (val == _getBlkSum(lo + 1) && lo + 1 < maxl) {
		return (lo + 1) * BLKSIZE;
	} else {
		_loadBlk(lo);
		ValueTp ret = lo * BLKSIZE + blk.rank(val - lsum);
		return ret;
	}
}

SDArrayBAux::ValueTp SDArrayBAux::rank2(ValueTp p, ValueTp &select) const {
	uint64_t v = rank(p);
	//TODO: optimize this
	select = prefixsum(v);
	return v;
}

void SDArrayBAux::clear() {
	blk.clear();
	header.clear();
	bits = nullptr;
	w1 = w2 = 0;
	sum = 0;
	len = 0;
}

void SDArrayBAux::load_aux(InpArchive &ar, const BitArrayInterface* ba) {
	bits = ba;
	ar.loadclass("SDArray_blk_aux");
	ar.var("length").load(len);
	ar.var("sum").load(sum);
	ar.var("w1").load(w1);
	ar.var("w2").load(w2);
	uint64_t bal;
	ar.var("store_bit_len").load(bal);
	header.load(ar.var("header"));
	ar.endclass();
	if (bal != bits->length()) throw ioerror("mismatch length");
	blk.clear();
}

void SDArrayBAux::save_aux(OutArchive &ar) const {
	ar.startclass("SDArray_blk_aux");
	ar.var("length").save(len);
	ar.var("sum").save(sum);
	ar.var("w1").save(w1);
	ar.var("w2").save(w2);
	uint64_t bal = bits->length();
	ar.var("store_bit_len").save(bal);
	header.save(ar.var("header"));
	ar.endclass();
}

void SDArrayCompress::load(InpArchive &ar) {
	ar.loadclass("SDArray_Compress");
	cbits.load(ar.var("compressed_bits"));
	load_aux(ar.var("aux"), &cbits);
	ar.endclass();
}

void SDArrayCompress::save(OutArchive &ar) const {
	ar.startclass("SDArray_Compress");
	cbits.save(ar.var("compressed_bits"));
	save_aux(ar.var("aux"));
	ar.endclass();
}

}//namespace 
