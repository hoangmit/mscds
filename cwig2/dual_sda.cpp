
#include "dual_sda.h"
#include "intarray/sdarray_sml.h"


struct NullSetGet: public mscds::SDABSetterInterface,
					public mscds::SDABGetterInterface {
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

	void width(uint8_t w) { blks.back().width = w; }
	void sum(uint64_t s) { blks.back().sum = s; }
	void hints(uint64_t h) { blks.back().hints = h; }

	void upper_len(unsigned int len) {
		buf = mscds::BitArray(len + 1);
		buf.fillzero();
	}

	void set_upper_pos(unsigned int p) { buf.setbit(p, true); }

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

	void loadBlk(unsigned int i) const { curblk = i; }
	uint64_t sum(unsigned int b) const { return blks[b].sum; }
	uint8_t width() const { return blks[curblk].width; }
	uint64_t hints() const { return blks[curblk].hints; }
	size_t upper_start() const { return blks[curblk].upper_pos; }

	uint64_t lower(unsigned int i) const {
		return lowers[curblk * SIZE + i];
	}
	//----------------------------------

	void length(uint64_t l) { _len = l; }
	void total_sum(uint64_t ts) { _total_sum = ts; }

	uint64_t blk_count() const { return blks.size(); }
	uint64_t length() const { return _len; }
	uint64_t total_sum() const { return _total_sum; }

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
