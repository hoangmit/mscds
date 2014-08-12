


#include "blkgroup_array.h"
#include "sdarray_block.h"

#include "bitarray/bitstream.h"

#include "fused_sdarray_test.h"
#include "sdarray_block.h"
#include "intarray/sdarray.h"

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
		assert(vals.size() == i);
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



void test_all1() {
	test1();
	test2();
	for (unsigned i = 0; i < 1000; i++)
		sdarray_block__test1();
}

int main(int argc, char* argv[]) {
	/*test_all1();
	test4();
	test_ptr();*/
	
	BenchmarkRegister::run_all_bm();
	return 0;
}

