#include "gamma_arr.h"
#include "huffarray.h"
#include "deltaarray.h"
#include "remap_dt.h"
#include "sdarray.h"
#include "sdarray_sml.h"
#include "utils/utest.h"
#include <vector>

namespace tests {

using namespace std;
using namespace mscds;

std::vector<unsigned int> gen_zeros(unsigned int n) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i) ret.push_back(0);
	return ret;
}

std::vector<unsigned int> gen_ones(unsigned int n) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i) ret.push_back(1);
	return ret;
}

std::vector<unsigned int> gen_zerosones(unsigned int n) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i)
		ret.push_back(i % 2);
	return ret;
}

std::vector<unsigned int> gen_inc(unsigned int n) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i)
		ret.push_back(i);
	return ret;
}

std::vector<unsigned int> gen_dec(unsigned int n) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i)
		ret.push_back(i);
	return ret;
}

std::vector<unsigned int> gen_rand(unsigned int n, unsigned int lower = 0,
	unsigned int higher = 100) {
	std::vector<unsigned int> ret;
	for (unsigned int i = 0; i < n; ++i)
		ret.push_back(lower +  rand() % (higher - lower));
	return ret;
}


template<typename QueryTp>
void testEnum(const std::vector<unsigned int>& exp, const QueryTp& sq) {
	assert(exp.size() > 3);
	for (unsigned int i = 0; i < 1000; i++) {
		unsigned int pos = rand() % (exp.size() - 3);
		typename QueryTp::Enum e;
		sq.getEnum(pos, &e);
		for (unsigned int j = 0; j < 3; ++j) {
			ASSERT_EQ(exp[pos + j], e.next());
		}
	}
}

template<typename QueryTp, typename BuilderTp>
void check(const std::vector<unsigned int>& exp, bool testenum = false) {
	BuilderTp bd;
	for (unsigned int v : exp) {
		bd.add(v);
	}
	QueryTp qs;
	bd.build(&qs);
	ASSERT_EQ(exp.size(), qs.length());
	for (unsigned int i = 0; i < exp.size(); ++i) {
		ASSERT_EQ(exp[i], qs.lookup(i)) << "i = " << i << endl;
	}
	for (int k = 0; k < 200; k++) {
		auto i = rand() % exp.size();
		ASSERT_EQ(exp[i], qs.lookup(i));
	}
	if (testenum) {
		testEnum(exp, qs);
	}
}


TEST(GammaArray, testsuite) {
	typedef GammaArray QueryTp;
	typedef GammaArrayBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);


	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(10000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}

TEST(DeltaArray, testsuite) {
	typedef DeltaCodeArr QueryTp;
	typedef DeltaCodeArrBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);

	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec, true);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(10000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}

TEST(DiffDeltaArray, testsuite) {
	typedef DiffDeltaArr QueryTp;
	typedef DiffDeltaArrBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);


	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec, true);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(10000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}

TEST(SDArraySml, testsuite) {
	typedef SDArraySml QueryTp;
	typedef SDArraySmlBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);

	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec, true);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(10000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}

}

TEST(HuffArray, testsuite) {
	typedef HuffmanArray QueryTp;
	typedef HuffmanArrBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);

	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(50000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}


TEST(HuffDiffArray, testsuite) {
	typedef HuffDiffArray QueryTp;
	typedef HuffDiffArrBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);

	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(50000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}


TEST(RemapDtArray, testsuite) {
	typedef RemapDtArray QueryTp;
	typedef RemapDtArrayBuilder BuilderTp;
	std::vector<unsigned int> vec;
	vec = gen_zeros(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zeros(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_ones(10000);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(50);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_zerosones(10000);
	check<QueryTp, BuilderTp>(vec);

	vec = gen_inc(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_dec(100);
	check<QueryTp, BuilderTp>(vec);
	vec = gen_rand(10000, 0, 100);
	check<QueryTp, BuilderTp>(vec);
	for (int i = 0; i < 5; ++i) {
		vec = gen_rand(50000, 0, 1000);
		check<QueryTp, BuilderTp>(vec, true);
	}
}

}//namespace

/*
int main(int argc, char* argv[]) {
	//::testing::GTEST_FLAG(filter) = "HuffArray.*";
	::testing::InitGoogleTest(&argc, argv); 
	int rs = RUN_ALL_TESTS();
	return rs;
}*/
