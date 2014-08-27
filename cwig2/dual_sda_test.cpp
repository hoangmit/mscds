
#include "dual_sda.h"
#include "utils/benchmark.h"
#include "fusion/sdarray_blk_hints.h"

#include "intarray/sdarray_sml.h"
#include "intarray/sdarray_th.h"
#include "intarray/sdarray_zero.h"

#include <vector>
#include <stdint.h>

using namespace std;
using namespace mscds;

void testx() {
	std::vector<unsigned int> df;
	unsigned int n = 2000, r = 500;
	for (unsigned int i = 0; i < n; ++i) {
		df.push_back(rand() % r);
	}
	test_dsdd(df);
	test_dfsd(df);
}


struct SDFix: public SharedFixtureItf {

	SDFix() {}

	unsigned int size, qsize, range, lowerrange;
	bool is_sort;

	uint64_t sum;

	void init() {
		size = 50000000;
		qsize = 2000000;
		range =      1000000;
		lowerrange = 1000000;
		is_sort = false;
	}

	void SetUp() {
		init();
		vals.resize(size);
		for (unsigned int i = 0; i < size; ++i) {
			vals[i] = lowerrange + rand() % range;
		}

		SDArraySmlBuilder stbd, edbd;
		LiftStBuilder<SDArrayFuseHintsBuilder, SDArrayFuseBuilder> bbbd;
		LiftStBuilder<SLG_Builder> dsdb;
		bbbd.g<0>().start_model();
		for (unsigned int i = 0; i < size; ++i)
			bbbd.g<0>().model_add(vals[i]);
		bbbd.g<0>().build_model();
		bbbd.init();
		dsdb.init();
		sum = 0;
		for (unsigned int i = 0; i < size; ++i) {
			stbd.add(vals[i]);
			edbd.add(vals[i] + 1);
			bbbd.g<0>().add(vals[i]);
			bbbd.g<1>().add(vals[i] + 1);
			bbbd.check_end_block();
			dsdb.g<0>().add(vals[i], vals[i] + 1);
			dsdb.check_end_block();
			sum += vals[i];
		}
		bbbd.check_end_data();
		dsdb.check_end_data();

		stbd.build(&st);
		edbd.build(&ed);
		bbbd.build(&bb);
		dsdb.build(&dualsda);
		generate_queries();
	}

	void generate_queries() {
		lookupqs.resize(qsize);
		queries = NULL;
		assert(qsize <= size);
		if (qsize == size) {
			for (unsigned i = 0; i < qsize; ++i)
				lookupqs[i] = i;
		} else {
			unsigned d = size  / qsize;
			for (unsigned i = 0; i < qsize - 1; ++i)
				lookupqs[i] = (unsigned)(d*i);
		}
		if (!is_sort)
			std::random_shuffle(lookupqs.begin(), lookupqs.end());
		queries = &lookupqs;
	}

	void TearDown() {
		dualsda.clear();
		bb.clear();
	}
	std::vector<unsigned> vals;
	std::vector<unsigned int> * queries;
	std::vector<unsigned int> lookupqs;

	SDArraySml st, ed;

	LiftStQuery<SDArrayFuseHints, SDArrayFuse> bb;
	LiftStQuery<SLG_Q> dualsda;
};

void sda_2xSDA_access(SDFix * fix) {
	auto& x = fix->st;
	auto& y = fix->ed;
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup((*qs)[i]);
		vx ^= y.lookup((*qs)[i]);
	}
}

void sda_fused_access(SDFix * fix) {
	auto& x = fix->bb.g<0>();
	auto& y = fix->bb.g<1>();
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup((*qs)[i]);
		vx ^= y.lookup((*qs)[i]);
	}
}

void sda_dual_access(SDFix * fix) {
	auto& x = fix->dualsda.g<0>().start;
	auto& y = fix->dualsda.g<0>().lg;
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup((*qs)[i]);
		vx ^= y.lookup((*qs)[i]);
	}
}



static const unsigned int RANDSEED = 3571;

BENCHMARK_SET(random_access_sda) {
	srand(RANDSEED);
	Benchmarker<SDFix> bm;
	bm.n_samples = 3;

	bm.add("sda_2xSDA_rnd_access", sda_2xSDA_access, 15);
	bm.add("sda_fused_rnd_access", sda_fused_access, 15);
	bm.add("sda_dual_rnd_access", sda_dual_access, 15);

	SDFix f;
	f.is_sort = false;
	bm.run_all(&f);
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(seq_access_sda) {
	srand(RANDSEED);
	Benchmarker<SDFix> bm;
	bm.n_samples = 3;

	bm.add("sda_2xSDA_seq_access", sda_2xSDA_access, 15);
	bm.add("sda_fused_seq_access", sda_fused_access, 15);
	bm.add("sda_dual_seq_access", sda_dual_access, 15);

	SDFix f;
	f.is_sort = true;
	bm.run_all(&f);
	bm.report(0); // <-- baseline
}


int main() {

	BenchmarkRegister::run_all_bm();

	testx();
	return 0;
}
