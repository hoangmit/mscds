
#include "fused_sdarray_test.h"
#include "block_mem_mng.h"
#include "inc_ptrs2.h"
#include "inc_ptrs3.h"
#include "utils/utest.h"

using namespace std;
using namespace mscds;

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
		uint8_t v = 1;
		bd.set_global(1, MemRange::wrap(v));
		v = 2;
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
		ASSERT_EQ(1, b1.v);
		br = mng.getSummary(1, 0);
		ASSERT_EQ(1, br.bits(0, br.len));

		b2.loadBlock(mng.getData(2, 0));
		ASSERT_EQ(3, b2.v);
		br = mng.getSummary(2, 0);
		ASSERT_EQ(2, br.bits(0, br.len));

		b1.loadBlock(mng.getData(1, 1));
		ASSERT_EQ(5, b1.v);
		br = mng.getSummary(1, 1);
		ASSERT_EQ(3, br.bits(0, br.len));

		b2.loadBlock(mng.getData(2, 1));
		ASSERT_EQ(7, b2.v);
		br = mng.getSummary(2, 1);
		ASSERT_EQ(4, br.bits(0, br.len));

		br = mng.getGlobal(1);
		ASSERT_EQ(1, br.bits(0, br.len));
		br = mng.getGlobal(2);
		ASSERT_EQ(2, br.bits(0, br.len));
	}
};

TEST(blk_mem, test1) {
	MockBigSt x;
	x.buildall();
	x.load_all();
}

void debug_run() {
	test_ptr();
}

int main(int argc, char* argv[]) {

	debug_run();

	::testing::GTEST_FLAG(catch_exceptions) = "0";
	::testing::GTEST_FLAG(break_on_failure) = "1";
	//::testing::GTEST_FLAG(filter) = "*.*";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;

	return 0;
}
