

#include "sdarray.h"
#include "sdarray_sml.h"
#include "sdarray_block.h"

#include "bitarray/bitstream.h"

#include "utils/benchmark.h"
#include "utils/str_utils.h"

#include "blkgroup_array.h"
#include "inc_ptrs2.h"
#include "inc_ptrs3.h"
#include <cstdlib>
#include <iostream>

using namespace mscds;

void sdarray_block__test1() {
	const unsigned n = 512; //block size, lol
	std::vector<uint64_t> inp;
	for (unsigned i = 0; i < n; ++i)
		inp.push_back(rand() % 100);

	OBitStream out;
	SDArrayBlock bdr;
	SDArrayBuilder bd;
	for (unsigned i = 0; i < n; ++i) {
		bdr.add(inp[i]);
		bd.add(inp[i]);
	}
	SDArrayQuery r;
	bd.build(&r);
	BitArray ba;
	bdr.saveBlock(&out);
	out.close();
	out.build(&ba);
	SDArrayBlock bdl;
	bdl.loadBlock(ba, 0, 0);

	for (unsigned i = 0; i < n; ++i) {
		auto l = r.lookup(i);
		assert(l == bdl.lookup(i));
		uint64_t ps1, ps2 = 0;
		ps1 = r.prefixsum(i);
		assert(ps1 == bdl.prefixsum(i));
		/*{
			std::cout << i << std::endl;
			std::cout << r.prefixsum(i) << "  " << bdl.prefixsum(i) << std::endl;
		}*/
		assert(l == bdl.lookup(i, ps2));
		assert(ps1 == ps2);
	}
	assert(r.prefixsum(n) == bdl.prefixsum(n));
}

struct MockBlk {
	const static uint64_t header = 0x0102030405060708ull;
	uint16_t v;

	void saveBlock(OBitStream * bs) {
		uint64_t v = header;
		bs->puts(v);
		v = this->v;
		bs->puts(v, 16);
	}

	void loadBlock(const BitRange& br) {
		loadBlock(*br.ba, br.start, br.len);
	}

	void loadBlock(const BitArray& ba, size_t pt, size_t len) {
		uint64_t x;
		assert(64 + 16 == len);
		x = ba.bits(pt, 64);
		if (x != header) {
			throw std::runtime_error("wrong load");
		}

		v = ba.bits(pt + 64, 16);
	}
};

struct MockBigSt {
	BlockBuilder bd;
	BlockMemManager mng;
	MockBlk b1, b2;

	void buildall() {
		unsigned int idx;
		idx = bd.register_summary(1, 2);
		assert(1 == idx);
		idx = bd.register_data_block();
		assert(1 == idx);
		idx = bd.register_summary(1, 2);
		assert(2 == idx);
		idx = bd.register_data_block();
		assert(2 == idx);
		
		bd.init_data();
		uint8_t v = 0;
		bd.set_global(1, MemRange::wrap(v));
		bd.set_global(2, MemRange::wrap(v));

		uint16_t tt;
		tt = 1;
		bd.set_summary(1, MemRange::wrap(tt));
		OBitStream& d1 = bd.start_data(1);
		b1.v = 1;
		b1.saveBlock(&d1);
		bd.end_data();

		tt = 2;
		bd.set_summary(2, MemRange::wrap(tt));
		OBitStream& d2 = bd.start_data(2);
		b2.v = 3;
		b2.saveBlock(&d2);
		bd.end_data();

		bd.end_block();
		//--------------------------------
		tt = 3;
		bd.set_summary(1, MemRange::wrap(tt));
		OBitStream& d3 = bd.start_data(1);
		b1.v = 5;
		b1.saveBlock(&d3);
		bd.end_data();

		tt = 4;
		bd.set_summary(2, MemRange::wrap(tt));
		OBitStream& d4 = bd.start_data(2);
		b2.v = 7;
		b2.saveBlock(&d4);
		bd.end_data();

		bd.end_block();
		//--------------------------------
		bd.build(&mng);
	}

	void load_all() {
		BitRange br;

		b1.loadBlock(mng.getData(1, 0));
		assert(1 == b1.v);
		br = mng.getSummary(1, 0);
		assert(1 == br.bits(0, br.len));

		b2.loadBlock(mng.getData(2, 0));
		assert(3 == b2.v);
		br = mng.getSummary(2, 0);
		assert(2 == br.bits(0, br.len));

		b1.loadBlock(mng.getData(1, 1));
		assert(5 == b1.v);
		br = mng.getSummary(1, 1);
		assert(3 == br.bits(0, br.len));

		b2.loadBlock(mng.getData(2, 1));
		assert(7 == b2.v);
		br = mng.getSummary(2, 1);
		assert(4 == br.bits(0, br.len));

	}
};

void test1() {
	MockBigSt x;
	x.buildall();
	x.load_all();
}


class TwoSDA_v1 {
public:
	TwoSDA_v1() {}
	BlockBuilder bd;
	BlockMemManager mng;

	size_t sum1, sum2;
	
	SDArrayBlock b1, b2;
	static const unsigned BLKSIZE = 512;

	size_t cnt, total;

	void init() {
		cnt = 0;
		total = 0;
		unsigned int idx;
		idx = bd.register_summary(16, 8);
		assert(idx == 1);
		idx = bd.register_data_block();
		assert(idx == 1);

		idx = bd.register_summary(16, 8);
		assert(idx == 2);
		bd.register_data_block();
		assert(idx == 2);
		bd.init_data();
	}

	void add(unsigned int x, unsigned int y) {
		b1.add(x);
		sum1 += x;
		b2.add(y);
		sum2 += y;
		++cnt;
		++total;
		if (cnt == BLKSIZE) {
			end_block();
		}
	}

	void end_block() {
		uint64_t v = sum1;
		bd.set_summary(1, MemRange::wrap(v)); 
		OBitStream& d1 = bd.start_data(1);
		b1.saveBlock(&d1);
		bd.end_data();

		v = sum2;
		bd.set_summary(2, MemRange::wrap(v));
		OBitStream& d2 = bd.start_data(2);
		b2.saveBlock(&d2);
		bd.end_data();

		bd.end_block();
		cnt = 0;
	}

	void build() {
		if (cnt > 0) end_block();
		struct {
			uint64_t cnt, sum;
		} data;
		data.cnt = cnt;
		data.sum = sum1;
		bd.set_global(1, MemRange::wrap(data));
		data.cnt = cnt;
		data.sum = sum2;
		bd.set_global(2, MemRange::wrap(data));
		bd.build(&mng);
	}
};


struct MockInterBlkQr: public InterBLockQueryTp {
	unsigned int sid, did;
	MockInterBlkQr(): mng(nullptr), sid(0), did(0) {}
	MockBlk blk;
	BlockMemManager * mng;

	void setup(BlockMemManager& _mng, StructIDList& lst) {
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

	void clear() { mng = nullptr; sid = 0; did = 0; }
};

struct MockInterBlkBd: public InterBlockBuilderTp {
	unsigned int cnt;
	MockInterBlkBd(BlockBuilder& bd_): bd(&bd_) {}
	MockInterBlkBd(): bd(nullptr) {}

	void init_bd(BlockBuilder& bd_) { bd = & bd_; }

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
		bd->set_summary(sid, MemRange::wrap(tt));
		OBitStream& d1 = bd->start_data(did);
		blk.v = cnt;
		blk.saveBlock(&d1);
		bd->end_data();
		cnt++;
	}

	void build_struct() {
		uint8_t v = 1;
		bd->set_global(sid, MemRange::wrap(v));
	}

	void deploy(StructIDList& lst) {
		lst.addId("mock");
		lst.add(sid);
		lst.add(did);
	}

	void clear() { sid = 0; did = 0; }

	MockBlk blk;
	unsigned int sid, did;
	BlockBuilder * bd;
};


struct TwoSDA_Query {
	BlockMemManager mng;
	void clear() { mng.clear(); mock.clear(); }
	MockInterBlkQr mock;
	SDArrayFuse x, y;

	void init(StructIDList& lst) {
		mock.setup(mng, lst);
		x.setup(mng, lst);
		y.setup(mng, lst);
	}
};

struct TwoSDA_Builder {
	BlockBuilder bd;
	MockInterBlkBd mbx;
	SDArrayFuseBuilder bd1, bd2;
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
		StructIDList lst;
		mbx.deploy(lst);
		bd1.deploy(lst);
		bd2.deploy(lst);
		out->init(lst);
	}
	unsigned int blkcntx;
};

void test2() {
	TwoSDA_Builder arr;
	arr.init();
	SDArrayBuilder t1, t2;
	std::vector<std::pair<unsigned, unsigned> > rawval;
	const unsigned int n = 1000;
	for (unsigned i = 0; i < n; ++i) {
		auto x = rand() % 100;
		auto y = rand() % 100;
		arr.add(x, y);
		t1.add(x);
		t2.add(y);
		rawval.push_back(std::make_pair(x, y));
	}

	SDArrayQuery q1, q2;
	t1.build(&q1);
	t2.build(&q2);

	TwoSDA_Query qs;
	arr.build(&qs);

	uint64_t ps1 = 0;
	uint64_t ps2 = 0;

	for (unsigned i = 0; i < n; ++i) {
		assert(rawval[i].first == qs.x.lookup(i));
		assert(rawval[i].second == qs.y.lookup(i));
		assert(ps1 == qs.x.prefixsum(i));
		assert(ps2 == qs.y.prefixsum(i));
		ps1 += rawval[i].first;
		ps2 += rawval[i].second;
	}
	assert(ps1 == qs.x.prefixsum(n));
	assert(ps2 == qs.y.prefixsum(n));
}

void test3() {
	typedef LiftStBuilder<MockInterBlkBd, SDArrayFuseBuilder, SDArrayFuseBuilder> TwoSDArrv3;
	TwoSDArrv3 bdx;
	typedef LiftStQuery<MockInterBlkQr, SDArrayFuse, SDArrayFuse> TwoSDA_v3_Query;
	TwoSDA_v3_Query qsx;
	bdx.init();
	bdx._end_block();
	bdx.build(&qsx);
}

#include "fuse_blk_model.h"

void test4() {
	const unsigned int n = 1001, r = 100;
	std::vector<unsigned int> vals;
	for (unsigned int i = 0; i < n; ++i)
		if (rand() % 1000 < 20)
			vals.push_back(2);
		else vals.push_back(rand() % r);

	LiftStBuilder<CodeInterBlkBuilder> bd;
	auto& x = bd.g<0>();
	x.start_model();
	for (unsigned i = 0; i < n; ++i)
		x.model_add(vals[i]);
	x.build_model();
	bd.init();
	for (unsigned i = 0; i < n; ++i) {
		//std::get<0>(bd.list).add(vals[0]);
		x.add(vals[i]);

		if (bd.is_all_full()) {
			bd._end_block();
		}
	}

	LiftStQuery<CodeInterBlkQuery> qs;
	if (!bd.is_all_empty()) {
		bd._end_block();
	}
	
	bd.build(&qs);
	auto& y = qs.g<0>();
	//y.debug_print(0);
	//y.debug_print(1);
	for (unsigned i = 0; i < n; ++i) {
		auto v = y.get(i);
		assert(vals[i] == v);
	}
	{
		unsigned i = 0;
		CodeInterBlkQuery::Enum e;
		y.getEnum(0, &e);
		while (e.hasNext()) {
			auto v = e.next();
			assert(i < vals.size());
			assert(vals[i] == v);
			++i;
		}
	}
	
}

void test_ptr() {
	const unsigned int n = 64, r = 100, st = 10;
	FixBlockPtr p1;
	AxPtr p2;
	MicroSDPtr p3;
	std::vector<unsigned int> start(n + 1);

	p1.init(n);
	p2.init(n);
	p3.init(n);
	start[0] = 0;
	for (unsigned int i = 1; i <= n; ++i) {
		unsigned v = rand() % r + st;
		p1.add(v);
		p2.add(v);
		p3.add(v);
		start[i] += start[i-1] + v;
	}
	p1._build();
	p2._build();
	p3._build();
	OBitStream os1, os2, os3;
	p1.saveBlock(&os1);
	uint8_t w2, w3;
	p2.saveBlock(&os2, &w2);

	p3.saveBlock(&os3, &w3);
	os3.close();

	BitArray b1, b2;
	os1.build(&b1);
	os2.build(&b2);
	//BitArray ba(b1);
	p1.reset();
	p2.reset();
	p1.loadBlock(b1, 0, b1.length());
	p2.loadBlock(b2, 0, b2.length(), w2);

	for (unsigned int i = 0; i <= n; ++i) {
		unsigned v1 = p1.start(i);
		unsigned v2 = p2.start(i);
		assert(start[i] == v1);
		assert(start[i] == v2);
	}

	std::cout << "Distance-bit: " << b1.length() << "   PS-bit: " << b2.length() << std::endl;

}
//--------------------------------------------------------------------------
// Benchmark

struct StmFix : public SharedFixtureItf {
	static const unsigned int SIZE = 2000000;
	void SetUp(int size) {
		if (size <= 0) { size = SIZE; }
		// generate test cases and data structure here
		unsigned int range = 500;

		SDArrayBuilder bd1;
		SDArraySmlBuilder bd2;
		TwoSDA_Builder xd;
		xd.init();

		uint64_t sum =0;
		
		for (unsigned i = 0; i < size; ++i) {
			unsigned val = rand() % range;
			vals.push_back(val);
			bd1.add(val);
			bd2.add(val);
			xd.add(val, val);
			sum += val;
		}
		bd1.build(&sd1);
		bd2.build(&sd2);
		xd.build(&qs);
		this->size = size;
		queries.resize(size);
		for (unsigned i = 0; i < size; ++i)
			queries[i] = i;
		std::random_shuffle(queries.begin(), queries.end());
		for (unsigned int i = 0; i < size; ++i)
			rankqs.push_back((((unsigned)(rand() % 256) << 8) | (rand() % 256)) % sum);
	}

	void TearDown() {vals.clear(); sd1.clear(); sd2.clear(); qs.clear();}

	unsigned size;
	std::vector<unsigned> vals;
	std::vector<unsigned int> queries;
	std::vector<unsigned int> rankqs;
	SDArrayQuery sd1;
	SDArraySml sd2;
	TwoSDA_Query qs;
};


void vector_null(StmFix * fix) {
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->vals[i];
	}
}

void sdarray_64(StmFix * fix) {
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd1.lookup(i);
	}
}

void sdarray_512(StmFix * fix) {
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd2.lookup(i);
	}
}

void sdarray_fuse0(StmFix * fix) {
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.x.lookup(i);
	}
}

void sdarray_fuse1(StmFix * fix) {
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.y.lookup(i);
	}
}

//-------------------------------------------------
void sdarray_64_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd1.lookup(fix->queries[i]);
	}
}

void sdarray_512_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd2.lookup(fix->queries[i]);
	}
}

void sdarray_fuse0_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.x.lookup(fix->queries[i]);
	}
}

void sdarray_fuse1_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.y.lookup(fix->queries[i]);
	}
}

//-------------------------------------------------
void sdarray_64_rnd_rank(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd1.find(fix->rankqs[i]);
	}
}

void sdarray_512_rnd_rank(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->sd2.rank(fix->rankqs[i]);
	}
}

void sdarray_fuse0_rnd_rank(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.x.rank(fix->rankqs[i]);
	}
}

void sdarray_fuse1_rnd_rank(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= fix->qs.y.rank(fix->rankqs[i]);
	}
}

BENCHMARK_SET(sdarray_rnd_benchmark) {
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE;
	bm.add_remark("number of queries for each run: " + utils::tostr(n));
	//bm.add("vector", vector_null, 100);
	bm.add("sdarray_b64_rnd_access", sdarray_64, 15);
	bm.add("sdarray_b512_rnd_access", sdarray_512, 15);
	bm.add("sdarray_fuse0_rnd_access", sdarray_fuse0, 15);
	bm.add("sdarray_fuse1_rnd_access", sdarray_fuse1, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_seq_benchmark) {
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE;
	bm.add_remark("number of queries for each run: " + utils::tostr(n));
	//bm.add("vector", vector_null, 100);
	bm.add("sdarray_b64_seq_access", sdarray_64, 15);
	bm.add("sdarray_b512_seq_access", sdarray_512, 15);
	bm.add("sdarray_fuse0_seq_access", sdarray_fuse0, 15);
	bm.add("sdarray_fuse1_seq_access", sdarray_fuse1, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_rnd_rank_benchmark) {
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE;
	bm.add_remark("number of queries for each run: " + utils::tostr(n));
	//bm.add("vector", vector_null, 100);
	bm.add("sdarray_b64_rnd_rank", sdarray_64_rnd_rank, 15);
	bm.add("sdarray_b512_rnd_rank", sdarray_512_rnd_rank, 15);
	bm.add("sdarray_fuse0_rnd_rank", sdarray_fuse0_rnd_rank, 15);
	bm.add("sdarray_fuse1_rnd_rank", sdarray_fuse1_rnd_rank, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}


void test_all1() {
	test1();
	test2();
	for (unsigned i = 0; i < 1000; i++)
		sdarray_block__test1();
}

int main(int argc, char* argv[]) {
	test_all1();
	test4();
	test_ptr();

	//BenchmarkRegister::run_all();
	
	return 0;
}

