
#include "dual_sda.h"
#include "intarray/sdarray_sml.h"
#include "fusion/generic_struct.h"

struct NullSetGet: public mscds::SDABSetterInterface, public mscds::SDABGetterInterface {
public:
	static const unsigned int SIZE = 512;
	NullSetGet() {
		blkcnt = 0;
	}
	void newblock() {
		blkcnt += 1;
		blks.push_back(BlkRecord());
		blks.back().lower_pos = lowers.size();
		blks.back().upper_pos = upper_out.length();
		assert(blkcnt == blks.size());
		lowers.resize(blkcnt * SIZE);
	}

	void width(uint8_t w) {
		blks.back().width = w;
	}

	void sum(uint64_t s) {
		blks.back().sum = s;
	}

	void hints(uint64_t h) {
		blks.back().hints = h;
	}

	void upper_len(unsigned int len) {
		buf = mscds::BitArray(len + 1);
		buf.fillzero();
	}

	void set_upper_pos(unsigned int p) {
		buf.setbit(p, true);
	}

	void lower(unsigned int i, uint64_t value) {
		lowers[i + (blkcnt - 1) * SIZE] = value;
	}

	void finishblock() {
		upper_out.append(buf);
		buf.clear();
	}

	void build() {
		upper_out.close();
		upper_out.build(&upper);
	}
	//---------------------------------------

	void loadBlk(unsigned int i) const {
		curblk = i;
	}

	uint64_t sum(unsigned int b) const {
		return blks[b].sum;
	}

	uint8_t width() const {
		return blks[curblk].width;
	}

	uint64_t hints() const {
		return blks[curblk].hints;
	}

	size_t upper_start() const {
		return blks[curblk].upper_pos;
	}

	uint64_t lower(unsigned int i) const {
		return lowers[curblk * SIZE + i];
	}
	//----------------------------------

	void length(uint64_t l) {
		_len = l;
	}
	void total_sum(uint64_t ts) {
		_total_sum = ts;
	}

	uint64_t blk_count() const {
		return blks.size();
	}
	uint64_t length() const {
		return _len;
	}
	uint64_t total_sum() const {
		return _total_sum;
	}

	struct BlkRecord {
		uint64_t sum;
		uint16_t width;
		uint64_t hints;
		size_t upper_len;
		size_t upper_pos;
		size_t lower_pos;
	};

	std::vector<BlkRecord> blks;
	std::vector<unsigned int> lowers;
	mscds::OBitStream upper_out;
	mscds::BitArray upper, buf;
	mutable unsigned int curblk;
	unsigned int blkcnt;
	uint64_t _total_sum, _len;

};

void test_dsdd(const std::vector<unsigned int>& lst) {
	NullSetGet sg;
	unsigned int n = lst.size();
	mscds::SDArrayBlockGBuilder<NullSetGet> bd(sg);

	const unsigned int BSZ = mscds::SDArrayBlockGBuilder<NullSetGet>::BLKSIZE;
	unsigned int i = 0;

	size_t ts = 0;
	for (unsigned int v : lst) {
		bd.add(v);
		ts += v;
		++i;
		if (i == BSZ) {
			bd.buildBlk();
			i = 0;
		}
	}
	if (i != 0)
		bd.buildBlk();

	sg.length(i);
	sg.total_sum(ts);
	sg.build();

	mscds::SDArrayBlockG<NullSetGet> qs(sg);
	qs.bind(&sg.upper);

	for (unsigned int i = 0; i < n; ++i) {
		qs.loadBlock(i / BSZ);
		auto v1 = qs.lookup(i % BSZ);
		assert(lst[i] == v1);
	}
}

//------------------------------------

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
		VByteStream::append(stream, cur_blk.lu1 - BLKSIZE);
		VByteStream::append(stream, cur_blk.lu2 - BLKSIZE);
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
		info.sum1 = mng->summary_word(sid, blk);
		info.r = mng->getData(did, blk);
		auto r = info.r;
		info.curblk = blk;

		info.sum2 = r.bits(0, 64);
		info.w1 = r.bits(64, 8);
		info.w2 = r.bits(64 + 8, 8);
		info.h1 = r.bits(64 + 16, 64);
		info.h2 = r.bits(64 + 16 + 64, 64);
		size_t pos = r.start + 64*3 + 16;
		info.lu1 = VByteStream::extract(*r.ba, pos) + BLKSIZE;
		info.lu2 = VByteStream::extract(*r.ba, pos) + BLKSIZE;

		info.up1ptr = pos;
		info.up2ptr = pos + info.lu1;
		info.llptr = info.up2ptr + info.lu2;
		start_blk.bind(r.ba);
		lg_blk.bind(r.ba);
	}

	size_t quick_sum1(unsigned int blk) {
		if (info.curblk == blk) return info.sum1;
		else return mng->summary_word(sid, blk);
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


void test_dfsd(const std::vector<unsigned int>& lst) {
	mscds::LiftStBuilder<mscds::SLG_Builder> bd;
	mscds::SDArraySmlBuilder bdx;

	bd.init();
	auto& z = bd.g<0>();

	for (unsigned int v : lst) {
		bdx.add(v);
		z.add(v, v + 1);
		bd.check_end_block();
	}

	bd.check_end_data();
	mscds::SDArraySml q2;
	bdx.build(&q2);
	mscds::LiftStQuery<mscds::SLG_Q> qs;
	bd.build(&qs);

	auto& x = qs.g<0>();
	for (unsigned int i = 0; i < lst.size(); ++i) {
		auto vx = x.start.lookup(i);
		assert(lst[i] == vx);
		vx = x.lg.lookup(i);
		assert(lst[i] + 1 == vx);
	}
	uint64_t s1 = 0, s2 = 0;
	for (unsigned int i = 0; i < lst.size(); ++i) {
		auto vx = x.start.prefixsum(i);
		assert(s1 == vx);
		vx = x.lg.prefixsum(i);
		assert(s2 == vx);

		s1 += lst[i];
		s2 += lst[i] + 1;
	}

	x.start.rank(100);
	x.lg.rank(100);
}
