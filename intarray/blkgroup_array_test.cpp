

#include "sdarray.h"
#include "sdarray_sml.h"
#include "sdarray_block.h"

#include "bitarray/bitstream.h"

#include "utils/benchmark.h"

#include "blkgroup_array.h"
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
	bdl.loadBlock(ba, 0);

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
}

struct MockBlk {
	const uint64_t header = 0x0102030405060708ull;
	uint16_t v = 0;

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

struct MockBlkBd {
	MockBlkBd(BlockBuilder& bd_) : bd(bd_) {}
	void register_struct() {
		bd.register_struct(1, 2);
	}

	void finish_block() {
		uint16_t tt;
		tt = 1;
		OBitStream& d1 = bd.start_struct(0, MemRange::wrap(tt));
		blk.v = 1;
		blk.saveBlock(&d1);
		bd.finish_struct();
	}

	void build() {
		bd.globalStructData().put0(8);
	}

	MockBlk blk;
	unsigned int id;
	BlockBuilder & bd;
};


struct MockBigSt {
	BlockBuilder bd;
	BlockMemManager mng;
	MockBlk b1, b2;

	void buildall() {
		bd.register_struct(1,2);
		bd.register_struct(1,2);
		
		bd.init_data();
		bd.globalStructData().put0(2 * 8);

		uint16_t tt;
		tt = 1;
		OBitStream& d1 = bd.start_struct(0, MemRange::wrap(tt));
		b1.v = 1;
		b1.saveBlock(&d1);
		bd.finish_struct();

		tt = 2;
		OBitStream& d2 = bd.start_struct(1, MemRange::wrap(tt));
		b2.v = 3;
		b2.saveBlock(&d2);
		bd.finish_struct();

		bd.finish_block();
		//--------------------------------
		tt = 3;
		OBitStream& d3 = bd.start_struct(0, MemRange::wrap(tt));
		b1.v = 5;
		b1.saveBlock(&d3);
		bd.finish_struct();

		tt = 4;
		OBitStream& d4 = bd.start_struct(1, MemRange::wrap(tt));
		b2.v = 7;
		b2.saveBlock(&d4);
		bd.finish_struct();

		bd.finish_block();
		//--------------------------------
		bd.build(&mng);
	}

	void load_all() {
		BitRange br;

		b1.loadBlock(mng.getData(0, 0));
		assert(1 == b1.v);
		br = mng.getSummary(0, 0);
		assert(1 == br.bits(0, br.len));

		b2.loadBlock(mng.getData(0, 1));
		assert(3 == b2.v);
		br = mng.getSummary(0, 1);
		assert(2 == br.bits(0, br.len));

		b1.loadBlock(mng.getData(1, 0));
		assert(5 == b1.v);
		br = mng.getSummary(1, 0);
		assert(3 == br.bits(0, br.len));

		b2.loadBlock(mng.getData(1, 1));
		assert(7 == b2.v);
		br = mng.getSummary(1, 1);
		assert(4 == br.bits(0, br.len));

	}
};

void test1() {
	MockBigSt x;
	x.buildall();
	x.load_all();
}


class TwoSDA {
public:
	TwoSDA() {}
	BlockBuilder bd;
	BlockMemManager mng;

	size_t sum1, sum2;
	
	SDArrayBlock b1, b2;
	const unsigned BLKSIZE = 512;

	size_t cnt, total;

	void init() {
		cnt = 0;
		total = 0;
		//global
		//  8 bytes for cnt,
		//  8 bytes for total sum
		//header
		//  8 bytes for sum
		bd.register_struct(16,8);
		bd.register_struct(16,8);
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
			finish_block();
		}
	}

	void finish_block() {
		uint64_t v = sum1;
		OBitStream& d1 = bd.start_struct(0, MemRange::wrap(v)); 
		b1.saveBlock(&d1);
		bd.finish_struct();

		v = sum2;
		OBitStream& d2 = bd.start_struct(1, MemRange::wrap(v));
		b2.saveBlock(&d2);
		bd.finish_struct();

		bd.finish_block();
		cnt = 0;
	}

	void build() {
		if (cnt > 0) finish_block();
		auto & a = bd.globalStructData();
		a.puts(cnt, 64);
		a.puts(sum1, 64);
		a.puts(cnt, 64);
		a.puts(sum2, 64);
		bd.build(&mng);
	}
};

struct TwoSDA_v2 {
	BlockBuilder bd;
	SDArrayFuseBuilder bd1, bd2;
	TwoSDA_v2() : bd1(bd), bd2(bd) {}

	void init() {
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
			finish_block();
			cnt = 0;
			blkcntx++;
		}
	}

	void finish_block() {
		bd1.finish_block();
		bd2.finish_block();
		bd.finish_block();
	}

	void build() {
		finish_block();
		bd1.build();
		bd2.build();
		bd.build(&mng);
	}
	unsigned int blkcntx;
	BlockMemManager mng;
	unsigned int cnt = 0;
	void clear() { mng.clear(); }
};

void test2() {
	TwoSDA_v2 arr;
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

	arr.build();

	SDArrayFuse x(arr.mng, 0);
	SDArrayFuse y(arr.mng, 1);
	SDArrayQuery q1, q2;
	t1.build(&q1);
	t2.build(&q2);
	for (unsigned i = 0; i < n; ++i) {
		assert(rawval[i].first == x.lookup(i));
		assert(rawval[i].second == y.lookup(i));
	}

	
}
//--------------------------------------------------------------------------


struct StmFix : public SharedFixtureItf {
	void SetUp(int size) {
		if (size <= 0) { size = 50000; }
		// generate test cases and data structure here
		unsigned int range = 500;

		SDArrayBuilder bd1;
		SDArraySmlBuilder bd2;
		sdx.init();
		
		for (unsigned i = 0; i < size; ++i) {
			unsigned val = rand() % range;
			vals.push_back(val);
			bd1.add(val);
			bd2.add(val);
			sdx.add(val, val);
		}
		bd1.build(&sd1);
		bd2.build(&sd2);
		sdx.build();
		this->size = size;
	}
	

	void TearDown() {vals.clear(); sd1.clear(); sd2.clear(); sdx.clear();}

	unsigned size;
	std::vector<unsigned> vals;
	SDArrayQuery sd1;
	SDArraySml sd2;
	TwoSDA_v2 sdx;
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
	SDArrayFuse x(fix->sdx.mng, 0);
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= x.lookup(i);
	}
}

void sdarray_fuse1(StmFix * fix) {
	SDArrayFuse x(fix->sdx.mng, 1);
	unsigned vx = 101;
	for (unsigned i = 0; i < fix->size; ++i) {
		vx ^= x.lookup(i);
	}
}

BENCHMARK_SET(sdarray_benchmark) {
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	//bm.add("vector", vector_null, 100);
	bm.add("sdarray(64)", sdarray_64, 20);
	bm.add("sdarray(512)", sdarray_512, 20);
	bm.add("sdarray_fuse0", sdarray_fuse0, 10);
	bm.add("sdarray_fuse1", sdarray_fuse1, 10);

	bm.run_all();
	bm.report(0); // <-- baseline
}

int main(int argc, char* argv[]) {
	test1();
	test2();
	BenchmarkRegister::run_all();
	return 0;
	for (unsigned i = 0; i < 1000; i++)
		sdarray_block__test1();
	return 0;
}
