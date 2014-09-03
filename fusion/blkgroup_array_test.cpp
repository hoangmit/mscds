
#include "block_mem_mng.h"
#include "intarray/sdarray_zero.h"
#include "intarray/sdarray_sml.h"
#include "sdarray_block.h"
#include "sdarray_blk_hints.h"

#include "bitarray/bitstream.h"

#include "fused_sdarray_test.h"
#include "sdarray_block.h"
#include "intarray/sdarray.h"

#include "utils/utest.h"

#include "utils/benchmark.h"
#include "utils/str_utils.h"

#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdint.h>
#include "codec_block.h"


namespace tests {

using namespace mscds;
using namespace std;

void sdarray_block_test1() {
	const unsigned n = 512 + rand() % 4; //block size, hardcoded, lol
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
		ASSERT_EQ(l, bdl.lookup(i));
		uint64_t ps1, ps2 = 0;
		ps1 = r.prefixsum(i);
		ASSERT_EQ(ps1, bdl.prefixsum(i));
		/*{
		std::cout << i << std::endl;
		std::cout << r.prefixsum(i) << "  " << bdl.prefixsum(i) << std::endl;
		}*/
		ASSERT_EQ(l, bdl.lookup(i, ps2));
		ASSERT_EQ(ps1, ps2);
	}
	ASSERT_EQ(r.prefixsum(n), bdl.prefixsum(n));
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
		bd.set_summary(1, ByteMemRange::ref(v));
		OBitStream& d1 = bd.start_data(1);
		b1.saveBlock(&d1);
		bd.end_data();

		v = sum2;
		bd.set_summary(2, ByteMemRange::ref(v));
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
		bd.set_global(1, ByteMemRange::ref(data));
		data.cnt = cnt;
		data.sum = sum2;
		bd.set_global(2, ByteMemRange::ref(data));
		bd.build(&mng);
	}
};

TEST(fusion, sda_block_test2) {
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
		ASSERT_EQ(rawval[i].first, qs.x.lookup(i));
		ASSERT_EQ(rawval[i].second, qs.y.lookup(i));
		ASSERT_EQ(ps1, qs.x.prefixsum(i));
		ASSERT_EQ(ps2, qs.y.prefixsum(i));
		ps1 += rawval[i].first;
		ps2 += rawval[i].second;
	}
	ASSERT_EQ(ps1, qs.x.prefixsum(n));
	ASSERT_EQ(ps2, qs.y.prefixsum(n));
}

TEST(fusion, rank_test3) {
	LiftStBuilder<SDArrayFuseBuilder, SDArrayFuseHintsBuilder> bd2;
	SDArraySmlBuilder bdx;

	std::vector<unsigned> vals;
	const unsigned int n = 1000;
	auto& norm = bd2.g<0>();
	auto& sda_h = bd2.g<1>();

	SDArrayZero zero;

	sda_h.start_model();
	for (unsigned i = 0; i < n; ++i) {
		vals.push_back(rand() % 100);
		sda_h.model_add(vals.back());
	}
	sda_h.build_model();
	bd2.init();
	for (unsigned i = 0; i < n; ++i) {
		zero.add(vals[i]);
		norm.add(vals[i]);
		sda_h.add(vals[i]);
		bdx.add(vals[i]);
		bd2.check_end_block();
	}
	bd2.check_end_data();

	SDArraySml sa;
	LiftStQuery<SDArrayFuse, SDArrayFuseHints> qs2;
	bdx.build(&sa);
	bd2.build(&qs2);
	auto& x = qs2.g<0>();
	auto& y = qs2.g<1>();

	auto last = zero.prefixsum(n);
	for (unsigned int p = 1; p < last; ++p) {
		auto v1 = zero.rank(p);
		auto v4 = sa.rank(p);
		auto v2 = x.rank(p);
		auto v3 = y.rank(p);
		ASSERT_EQ(v1, v2);
		ASSERT_EQ(v1, v3);
		ASSERT_EQ(v1, v4);
	}

}

void test3() {
	typedef LiftStBuilder<MockInterBlkBd, SDArrayFuseBuilder, SDArrayFuseBuilder> TwoSDArrv3;
	TwoSDArrv3 bdx;
	typedef LiftStQuery<MockInterBlkQr, SDArrayFuse, SDArrayFuse> TwoSDA_v3_Query;
	TwoSDA_v3_Query qsx;
	bdx.init();
	bdx.check_end_data();
	bdx.build(&qsx);
}


TEST(fusion, codex_block) {
	const unsigned int n = 1001, r = 100;
	std::vector<unsigned int> vals;
	for (unsigned int i = 0; i < n; ++i)
		if (rand() % 1000 < 20)
			vals.push_back(2);
		else vals.push_back(rand() % r);

		LiftStBuilder<HuffBlkBuilder> bd;
		auto& x = bd.g<0>();
		x.start_model();
		for (unsigned i = 0; i < n; ++i)
			x.model_add(vals[i]);
		x.build_model();
		bd.init();
		for (unsigned i = 0; i < n; ++i) {
			//std::get<0>(bd.list).add(vals[0]);
			x.add(vals[i]);
			bd.check_end_block();
		}

		LiftStQuery<HuffBlkQuery> qs;
		bd.check_end_data();

		bd.build(&qs);
		auto& y = qs.g<0>();
		//y.debug_print(0);
		//y.debug_print(1);
		for (unsigned i = 0; i < n; ++i) {
			auto v = y.get(i);
			if (vals[i] != v) {
				v = y.get(i);
			}
			ASSERT_EQ(vals[i], v);
		}
	{
		unsigned i = 0;
		HuffBlkQuery::Enum e;
		y.getEnum(0, &e);
		while (e.hasNext()) {
			auto v = e.next();
			ASSERT(i < vals.size());
			ASSERT_EQ(vals[i], v);
			++i;
		}
		ASSERT_EQ(vals.size(), i);
	}
}


TEST(fusion, sda_block) {
	for (unsigned i = 0; i < 1000; i++)
		sdarray_block_test1();
}

void debug_cases() {
	test3();
}

}//namespace

using namespace tests;

int main(int argc, char* argv[]) {
	
	//debug_cases();

	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	::testing::GTEST_FLAG(filter) = "fusion.rank_test3";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	//return rs;

	//sdarray benchmark is in another file
	BenchmarkRegister::run_all_bm();
	return 0;
}

