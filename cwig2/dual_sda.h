#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"
#include "fusion/generic_struct.h"
#include <stdint.h>
#include <vector>

namespace mscds {

class SDABGetterInterface {
public:
	virtual void loadBlk(unsigned int i) const = 0;
	virtual uint8_t width() const = 0;
	virtual uint64_t hints() const = 0;
	virtual size_t upper_start() const = 0;
	virtual uint64_t lower(unsigned int i) const = 0;

//----------------

	virtual uint64_t sum(unsigned int b) const = 0;
	virtual uint64_t blk_count() const = 0;
	virtual uint64_t length() const = 0;
	virtual uint64_t total_sum() const = 0;
};

class SDABSetterInterface {
public:
	virtual void newblock() = 0;
	virtual void sum(uint64_t) = 0;
	virtual void width(uint8_t) = 0;
	virtual void hints(uint64_t) = 0;

	virtual void upper_len(unsigned int len) = 0;
	virtual void set_upper_pos(unsigned int p) = 0;

	virtual void lower(unsigned int i, uint64_t value) = 0;
	virtual void finishblock() = 0;

	virtual void length(uint64_t l) = 0;
	virtual void total_sum(uint64_t ts) = 0;
};

template<typename S>
class SDArrayBlockGBuilder {
public:
	SDArrayBlockGBuilder(S& setter_): setter(setter_) {}
	void add(unsigned int val) { vals.push_back(val); }
	void buildBlk();
	static const unsigned int BLKSIZE = 512;
private:
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;
	S& setter;
	std::vector<uint64_t> vals;
};

template<typename G>
class SDArrayBlockG {
public:
	typedef uint64_t ValueType;
	SDArrayBlockG(const G& getter_): getter(getter_), bits(NULL), lastblk(~0ull), select_hints(0), upper_pos(0) {}
	void bind(BitArray* bits_) { bits = bits_; }
	void loadBlock(unsigned int id);
	ValueType prefixsum(unsigned int  p) const;
	ValueType lookup(unsigned int p) const;
	ValueType lookup(unsigned int p, ValueType& prev_sum) const;
	ValueType blk_sum(unsigned int b) const;
	unsigned int rank(ValueType val) const;
	static const unsigned int BLKSIZE = 512;

	const G& raw() const { return getter; }
private:
	unsigned int select_hi(uint64_t hints, uint32_t off) const;
	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
	unsigned int select_zerohi(uint64_t hints, uint32_t off) const;
	uint64_t lower(size_t off) const;
private:
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;
private:
	const G& getter;
	uint16_t width;
	uint64_t select_hints;
	mscds::BitArray* bits;
	size_t upper_pos;
	size_t lastblk;
};

template<typename B>
class SDArrayInterBlkG {
public:
	typedef uint64_t ValueType;
	B& blk;
	SDArrayInterBlkG(B& b): blk(b) {}
	static const unsigned int BLKSIZE = 512;

	void loadBlk(unsigned int b) const {
		blk.loadBlock(b);
	}

	size_t getBlkSum(unsigned int b) const {
		return blk.blk_sum(b);
	}

	size_t total_sum() const {
		return blk.raw().total_sum();
	}

	ValueType prefixsum(unsigned int p) const {
		//if (p == this->len) return this->sum;
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		auto sum = getBlkSum(bpos);
		if (off == 0) return sum;
		else {
			loadBlk(bpos);
			return sum + blk.prefixsum(off);
		}
	}

	ValueType lookup(unsigned int p) const {
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		loadBlk(bpos);
		return blk.lookup(off);
	}

	size_t length() const { return blk.raw().length(); }

	size_t blk_count() const { return blk.raw().blk_count(); }

	ValueType lookup(unsigned int p, ValueType &prev_sum) const {
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		auto sum = getBlkSum(bpos);
		loadBlk(bpos);
		auto v = blk.lookup(p, prev_sum);
		prev_sum += sum;
		return v;
	}

	unsigned int rank(ValueType val) const {
		if (val > total_sum()) return length();
		uint64_t lo = 0;
		uint64_t hi = blk_count();
		while (lo < hi) {
			uint64_t mid = lo + (hi - lo) / 2;
			if (getBlkSum(mid) < val) lo = mid + 1;
			else hi = mid;
		}
		if (lo == 0) return 0;
		lo--;
		assert(val > getBlkSum(lo));
		loadBlk(lo);
		ValueType ret = lo * BLKSIZE + blk.rank(val - getBlkSum(lo));
		return ret;
	}

	unsigned int _rank(ValueType val, unsigned int begin, unsigned int end) const {
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
		loadBlk(lo);
		ValueType ret = lo * BLKSIZE + blk.rank(val - getBlkSum(lo));
		return ret;
	}
};

}//namespace

//-------------------------------------------------------------

namespace mscds {

template<typename S>
void SDArrayBlockGBuilder<S>::buildBlk() {
	assert(vals.size() <= BLKSIZE);
	if (vals.size() == 0) return;
	setter.newblock();
	while (vals.size() < BLKSIZE) vals.push_back(0);

	for (size_t p = 1; p < vals.size(); ++p)
		vals[p] += vals[p - 1];
	uint64_t width = ceillog2(1 + vals.back() / vals.size());
	assert(width < (1ULL << 7));

	//higher bits' hints
	uint64_t select_hints = 0;
	const unsigned int step = (BLKSIZE + SUBB_PER_BLK - 1) / SUBB_PER_BLK;
	size_t i = step;
	for (size_t p = 0; p < SUBB_PER_BLK - 1; ++p) {
		uint64_t hp = ((vals[i - 1] >> width) + i - 1);
		assert(ceillog2(hp) <= 10);
		select_hints |= (hp << (p * 10));
		assert(p * 10 <= 64);
		i += step;
	}
	setter.width(width);
	setter.hints(select_hints);

	//lower bits
	for (size_t p = 0; p < vals.size(); ++p)
		setter.lower(p, vals[p] & ((1ull << width) - 1));

	//higer bits
	setter.upper_len((vals[vals.size() - 1] >> width) + vals.size() - 1);
	size_t j = 0;
	for (size_t p = 0; p < vals.size(); p++) {
		size_t pos = (vals[p] >> width) + p;
		setter.set_upper_pos(pos);
	}
	vals.clear();
	setter.finishblock();
}

template<typename G>
void SDArrayBlockG<G>::loadBlock(unsigned int id) {
	if (id != lastblk) {
		getter.loadBlk(id);
		width = getter.width();
		select_hints = getter.hints();
		upper_pos = getter.upper_start();
		lastblk = id;
	}
}

template<typename G>
uint64_t SDArrayBlockG<G>::lower(size_t off) const {
	return getter.lower(off);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::blk_sum(unsigned int b) const {
	return getter.sum(b);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::prefixsum(unsigned int p) const {
	if (p == 0) return 0;
	ValueType lo = (width > 0) ? lower(p-1) : 0;
	ValueType hi = select_hi(select_hints, p - 1) + 1 - p;
	return ((hi << width) | lo);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::lookup(unsigned int off) const {
	uint64_t prev = 0;
	int64_t prehi = 0;
	if (off > 0) {
		ValueType prelo = lower(off - 1);
		prehi = select_hi(select_hints, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = lower(off);
	ValueType hi = prehi + bits->scan_next(upper_pos + prehi + off);
	ValueType cur = ((hi << width) | lo);
	return cur - prev;
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::lookup(unsigned int off, ValueType &prev_sum) const {
	ValueType prev = 0;
	ValueType prehi = 0;
	if (off > 0) {
		ValueType prelo = lower(off - 1);
		prehi = select_hi(select_hints, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = lower(width);
	ValueType hi = prehi + bits->scan_next(upper_pos + prehi + off);
	ValueType cur = ((hi << width) | lo);
	prev_sum = prev;
	return cur - prev;
}

template<typename G>
unsigned int SDArrayBlockG<G>::rank(ValueType val) const {
	ValueType vlo = val & ((1ull << width) - 1);
	ValueType vhi = val >> width;
	uint32_t hipos = 0, rank = 0;
	if (vhi > 0) {
		hipos = select_zerohi(select_hints, vhi - 1) + 1;
		rank = hipos - vhi;
	}
	ValueType curlo = 0;
	while (rank < BLKSIZE && bits->bit(upper_pos + hipos)) {
		curlo = lower(rank);
		if (curlo >= vlo)
			return rank + 1;
		++rank;
		++hipos;
	}
	return rank + 1;
}

template<typename G>
unsigned int SDArrayBlockG<G>::select_hi(uint64_t hints, uint32_t off) const {
	unsigned int subblkpos = off / SUBB_SIZE;
	uint32_t res = off % SUBB_SIZE;
	if (res == SUBB_SIZE - 1)
		return getBits(hints, subblkpos * 10, 10);
	unsigned int gb = subblkpos > 0 ? getBits(hints, (subblkpos - 1) * 10, 10) + 1 : 0;
	return bits->scan_bits(upper_pos + gb, res) + gb;
}

template<typename G>
uint64_t SDArrayBlockG<G>::getBits(uint64_t x, uint64_t beg, uint64_t num) {
	return (x >> beg) & ((1ULL << num) - 1);
}

template<typename G>
unsigned int SDArrayBlockG<G>::select_zerohi(uint64_t hints, uint32_t off) const {
	uint64_t sblk = 0;
	for (; sblk < 6; ++sblk) {
		uint64_t sbpos = getBits(hints, sblk * 10, 10);
		if (sbpos - (sblk + 1) * SUBB_SIZE + 1 >= off) break;
	}
	unsigned int res = off, sbpos = 0;
	if (sblk > 0) {
		sbpos = getBits(hints, (sblk - 1) * 10, 10) + 1;
		res -= sbpos - sblk * SUBB_SIZE;
	}
	return sbpos + bits->scan_zeros(upper_pos + sbpos, res);
}

}//namespace



namespace mscds {

struct SLG_Builder {
	static const unsigned int BLKSIZE = 512;
	struct {
		uint8_t w1, w2;
		uint64_t h1, h2;
		unsigned int lu1, lu2;
		//bool len_or_gap;
		BitArray u1, u2;

		std::vector<uint64_t> lower1, lower2;
	} cur_blk;

	BlockBuilder * bd;

	struct Start_setter: public mscds::SDABSetterInterface {
		SLG_Builder& p;
		// SetterInterface interface
	public:
		Start_setter(SLG_Builder& p_): p(p_) {}
		void newblock() {}
		void width(uint8_t w) { p.cur_blk.w1 = w; }
		void sum(uint64_t s) {}
		void hints(uint64_t h) { p.cur_blk.h1 = h; }
		void upper_len(unsigned int len) { p.cur_blk.u1.fillzero(); p.cur_blk.lu1 = len + 1; }
		void set_upper_pos(unsigned int px) { p.cur_blk.u1.setbit(px, true); }
		void lower(unsigned int i, uint64_t value) { p.cur_blk.lower1[i] = value; }
		void finishblock() {}

		void length(uint64_t l) {}
		void total_sum(uint64_t ts) {}
	} _stst;

	struct LG_setter: public mscds::SDABSetterInterface {
		SLG_Builder& p;
		// SetterInterface interface
	public:
		LG_setter(SLG_Builder& p_): p(p_) {}
		void newblock() {}
		void width(uint8_t w) { p.cur_blk.w2 = w; }
		void sum(uint64_t s) { }
		void hints(uint64_t h) { p.cur_blk.h2 = h; }
		void upper_len(unsigned int len) { p.cur_blk.u2.fillzero(); p.cur_blk.lu2 = len + 1; }
		void set_upper_pos(unsigned int px) { p.cur_blk.u2.setbit(px, true); }
		void lower(unsigned int i, uint64_t value) { p.cur_blk.lower2[i] = value; }
		void finishblock() {}

		void length(uint64_t l) {}
		void total_sum(uint64_t ts) {}
	} _lgst;

	unsigned int cnt;

	SDArrayBlockGBuilder<Start_setter> st;
	SDArrayBlockGBuilder<LG_setter> lg;
	SLG_Builder(): _stst(*this), _lgst(*this), st(_stst), lg(_lgst), cnt(0), sum1(0), sum2(0), total(0), rsum1(0), rsum2(0) {}

	uint64_t sum1, sum2, rsum1, rsum2;

	void add(unsigned int v1, unsigned int v2) {
		st.add(v1);
		lg.add(v2);
		rsum1 += v1;
		rsum2 += v2;
		cnt++;
		total++;
	}

	void init_buff() {
		cur_blk.u1 = BitArray(3*BLKSIZE + 1);
		cur_blk.u2 = BitArray(3*BLKSIZE + 1);
		cur_blk.lower1.resize(BLKSIZE);
		cur_blk.lower2.resize(BLKSIZE);
	}

	void init_bd(mscds::BlockBuilder& bd_) { bd = &bd_; init_buff(); }

	void register_struct() {
		bd->begin_scope("dual_sda");
		sid = bd->register_summary(8*3, 8);
		did = bd->register_data_block();
		bd->end_scope();
	}

	void send(const BitArray& ba, unsigned int len, OBitStream& stream) {
		const unsigned int WORDSIZE = 64;
		unsigned int p = 0, i = 0;
		while (p + WORDSIZE <= len) {
			stream.puts(ba.word(i));
			p += WORDSIZE;
			i += 1;
		}
		if (p < len) stream.puts(ba.word(i), len - p);
	}

	void set_block_data(bool lastblock = false) {
		st.buildBlk();
		lg.buildBlk();
		OBitStream& stream = bd->start_data(did);
		stream.puts(sum2);
		stream.puts(cur_blk.w1, 8);
		stream.puts(cur_blk.w2, 8);
		stream.puts(cur_blk.h1, 64);
		stream.puts(cur_blk.h2, 64);
		stream.puts(cur_blk.lu1 - BLKSIZE, 11);
		stream.puts(cur_blk.lu2 - BLKSIZE, 11);
		send(cur_blk.u1, cur_blk.lu1, stream);
		send(cur_blk.u2, cur_blk.lu2, stream);
		for (unsigned int i = 0; i < BLKSIZE; ++i) {
			stream.puts(cur_blk.lower1[i], cur_blk.w1);
			stream.puts(cur_blk.lower2[i], cur_blk.w2);
		}

		bd->end_data();
		bd->set_summary(sid, ByteMemRange::ref(sum1));
		cnt = 0;
		sum1 = rsum1;
		sum2 = rsum2;
	}

	bool is_empty() const {return cnt == 0; }
	bool is_full() const { return cnt >= BLKSIZE; }


	void build_struct() {
		struct {
			uint64_t total, sum1, sum2;
		} x;
		x.total = total;
		x.sum1 = sum1;
		x.sum2 = sum2;

		bd->set_global(sid, ByteMemRange::ref(x));
	}

	void deploy(mscds::StructIDList& lst) {
		lst.addId("dual_sda");
		lst.add(sid);
		lst.add(did);
	}
public:
	unsigned int sid, did;
	size_t total;
};

struct SLG_Q {
	static const unsigned int BLKSIZE = 512;
	BlockMemManager* mng;
	unsigned int sid, did;

	uint64_t _len, _sum1, _sum2;

	void setup(mscds::BlockMemManager& mng_, mscds::StructIDList& lst) {
		mng = &mng_;
		lst.checkId("dual_sda");
		sid = lst.get();
		did = lst.get();
		assert(sid > 0);
		assert(did > 0);
		//load_global();

		auto r = mng->getGlobal(sid);
		_len = r.word(0);
		_sum1 = r.word(1);
		_sum2 = r.word(2);
	}

	void clear() { mng->clear(); }
	void inspect(const std::string &cmd, std::ostream &out) {}

	struct {
		uint64_t sum1, sum2;
		uint8_t w1, w2;
		uint64_t h1, h2;
		unsigned int lu1, lu2;

		unsigned int up1ptr, up2ptr, llptr;
		
		unsigned int curblk;
		BitRange r;
	} info;

	void pre_load(unsigned int blk) {
		if (info.curblk == blk) return ;
		info.sum1 = mng->summary_word(sid, blk) & ((1ull << 63) - 1);
		info.r = mng->getData(did, blk);
		auto r = info.r;
		info.curblk = blk;

		info.sum2 = r.bits(0, 64);
		info.w1 = r.bits(64, 8);
		info.w2 = r.bits(64 + 8, 8);
		info.h1 = r.bits(64 + 16, 64);
		info.h2 = r.bits(64 + 16 + 64, 64);
		size_t pos = r.start + 64*3 + 16;
		info.lu1 = r.ba->bits(pos, 11) + BLKSIZE;
		info.lu2 = r.ba->bits(pos + 11, 11) + BLKSIZE;
		pos += 22;
		info.up1ptr = pos;
		info.up2ptr = pos + info.lu1;
		info.llptr = info.up2ptr + info.lu2;
		start_blk.bind(r.ba);
		lg_blk.bind(r.ba);
	}

	size_t quick_sum1(unsigned int blk) {
		if (info.curblk == blk) return info.sum1;
		else return mng->summary_word(sid, blk) & ((1ull << 63) - 1);
	}

	size_t slow_sum2(unsigned int blk) {
		if (info.curblk == blk) return info.sum2;
		auto r = mng->getData(did, blk);
		return r.word(0);
	}

	struct Start_getter: public mscds::SDABGetterInterface {
	// GetterInterface interface
		SLG_Q& pa;
	public:
		Start_getter(SLG_Q& ref): pa(ref) {}
		void loadBlk(unsigned int i) const { pa.pre_load(i); }
		uint8_t width() const { return pa.info.w1; }
		uint64_t sum(unsigned int b) const { return pa.quick_sum1(b); }
		uint64_t hints() const { return pa.info.h1; }
		size_t upper_start() const { return pa.info.up1ptr;}
		uint64_t lower(unsigned int i) const {
			return pa.info.r.ba->bits(pa.info.llptr + (pa.info.w1 + pa.info.w2)*i, pa.info.w1);
		}
		uint64_t blk_count() const { return (pa._len + BLKSIZE - 1) / BLKSIZE; }
		uint64_t length() const { return pa._len; }
		uint64_t total_sum() const { return pa._sum1; }
	} _stgt;

	struct LG_getter: public mscds::SDABGetterInterface {
	// GetterInterface interface
		SLG_Q& pa;
	public:
		LG_getter(SLG_Q& ref): pa(ref) {}
		void loadBlk(unsigned int i) const { pa.pre_load(i); }
		uint8_t width() const { return pa.info.w2; }
		uint64_t sum(unsigned int b) const { return pa.slow_sum2(b); }
		uint64_t hints() const { return pa.info.h2; }
		size_t upper_start() const { return pa.info.up2ptr; }
		uint64_t lower(unsigned int i) const {
			return pa.info.r.ba->bits(pa.info.llptr + (pa.info.w1 + pa.info.w2)*i + pa.info.w1, pa.info.w2);
		}
		uint64_t blk_count() const { return (pa._len + BLKSIZE - 1) / BLKSIZE; }
		uint64_t length() const { return pa._len; }
		uint64_t total_sum() const { return pa._sum2; }
	} _lggt;

	SDArrayBlockG<Start_getter> start_blk;
	SDArrayBlockG<LG_getter> lg_blk;

	SDArrayInterBlkG<SDArrayBlockG<Start_getter> > start;
	SDArrayInterBlkG<SDArrayBlockG<LG_getter> > lg;
	
	SLG_Q(): _stgt(*this), _lggt(*this), start_blk(_stgt), 
		lg_blk(_lggt), start(start_blk), lg(lg_blk)
	{ info.curblk = ~0u; }

};

}//namespace

namespace tests {
void test_dsdd(const std::vector<unsigned int>& lst);
void test_dfsd(const std::vector<unsigned int>& lst);
}

