#include "select_dense.h"
#include "select_dense_block.h"

#include "utils/utest.h"

namespace tests {

using namespace mscds;
using namespace std;

static std::vector<unsigned int> generate_list(unsigned int bvlen, double density) {
	assert(density <= 1.0 && density >= 0);
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < bvlen; ++i)
		if (rand() * 1.0 / RAND_MAX <= density)
			ret.push_back(i);
	ret.push_back(bvlen);
	return ret;
}


static void test_bit(double density) {
	const unsigned bxl = 800;
	auto v = generate_list(1.0 / density * bxl, density);
	if (v.size() > 512) v.resize(512);
	DenseSelectBlock bx;
	bx.h.v1 = 0;
	OBitStream out;
	bx.build(v, out, 0, 0);
}

static void test_store_case(unsigned int casex) {
	unsigned int len;
	if (casex == 0) len = 7;
	else len = 1u << (casex + 2);
	std::vector<unsigned int> vals(len);
	vals[0] = 0;
	for (unsigned int i = 1; i < vals.size(); i++) {
		vals[i] = rand() % 2048;
	}

	DenseSelectBlock bx;
	bx.h.v1 = 0;
	OBitStream out;
	bx._write_case(casex, 11, vals, out, 0);
	out.close();
	BitArray b;
	out.build(&b);
	for (unsigned int i = 0; i < vals.size(); i++) {
		unsigned v = bx.get_casex(i, b, 0);
		assert(vals[i] == v);
	}
}

TEST(select_dense, store_all) {
	for (unsigned int i = 0; i < 2000; ++i) {
		for (unsigned int casex = 0; i < 8; ++i) {
			test_store_case(casex);
		}
	}
}

void select_dense_bit_all() {
	test_bit(1.0);
	test_bit(0.75);
	test_bit(0.5);
	test_bit(0.25);
}

BitArray gen_bits(unsigned int len, double density) {
	OBitStream o;
	for (unsigned int i = 0; i < len; ++i) {
		if (rand() * 1.0 / RAND_MAX <= density)
			o.put1();
		else o.put0();
	}
	o.close();
	BitArray b;
	o.build(&b);
	return b;
}

static void test_gen(unsigned int len, double density) {
	BitArray b = gen_bits(len, density);
	unsigned cnt = 0;
	for (unsigned int i = 0; i < b.length(); ++i)
		if (b[i]) cnt++;
	SelectDense qs;
	SelectDenseBuilder::build(b, &qs);

	int lastpos = -1;
	for (unsigned int i = 0; i < cnt; ++i) {
		int pos = qs.select(i);
		if (!(lastpos < pos)) {
			pos = qs.select(i);
		}
		assert(lastpos < pos);
		lastpos = pos;
		assert(b[pos] == true);
	}
}

TEST(select_dense, all) {
	test_gen(2000, 1.0);
	test_gen(2000, 0.75);
	test_gen(2000, 0.5);
	test_gen(2000, 0.25);
	test_gen(2000, 0.15);

	test_gen(2048, 1.0);
	test_gen(2048, 0.75);
	test_gen(2048, 0.5);
	test_gen(2048, 0.25);
	test_gen(2048, 0.15);

}

}//namespace

/*
int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "*";
	//::testing::GTEST_FLAG(break_on_failure) = "1";
	//::testing::GTEST_FLAG(catch_exceptions) = "0";

	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
}*/
