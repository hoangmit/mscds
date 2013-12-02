#include "utils/benchmark.h"
#include "rank25p.h"
#include "rank6p.h"
#include "rank3p.h"
#include "rrr.h"
#include "utils/utils.h"
using namespace std;
using namespace mscds;

struct RankBMFix : public SharedFixtureItf {
	void SetUp(int size) {
		if (size <= 0) {
			size = 50000000;
		}
		size_t onepc = 5000;
		size_t queries_cnt = 100000;
		ba = BitArray::create(size);
		for (int i = 0; i < size; ++i)
			if (rand() % 10000 < onepc) ba.setbit(i, true);
			else ba.setbit(i, false);
		queries.clear();
		queries.resize(queries_cnt);
		for (unsigned int j = 0; j < queries_cnt; ++j) {
			queries.push_back(utils::rand32() % size);
		}
		Rank25pBuilder br25;
		br25.build(ba, &r25);

		Rank6pBuilder br6;
		br6.build(ba, &r6);

		Rank3pBuilder br3;
		br3.build(ba, &r3);

		RRRBuilder brr;
		brr.build(ba, &rr);
	}

	void TearDown() {
		queries.clear();
		ba.clear();
		r25.clear();
		r6.clear();
		r3.clear();
		rr.clear();
	}

	BitArray ba;
	vector<unsigned int> queries;
	Rank25p r25;
	Rank6p r6;
	Rank3p r3;
	RRR rr;
};

void rankbm_null(RankBMFix * fix) {
	unsigned int t = 0;
	for (auto p : fix->queries) {
		t ^= 101;
		t >>= 1;
	}
}

void rankbm_rank25(RankBMFix * fix) {
	unsigned int t = 0;
	for (auto p : fix->queries) {
		t ^= fix->r25.rank(p);
	}
}

void rankbm_rank6(RankBMFix * fix) {
	unsigned int t = 0;
	for (auto p : fix->queries) {
		t ^= fix->r6.rank(p);
	}
}

void rankbm_rank3(RankBMFix * fix) {
	unsigned int t = 0;
	for (auto p : fix->queries) {
		t ^= fix->r3.rank(p);
	}
}

void rankbm_rankrr(RankBMFix * fix) {
	unsigned int t = 0;
	for (auto p : fix->queries) {
		t ^= fix->rr.rank(p);
	}
}

BENCHMARK_SET(rank_benchmark) {
	Benchmarker<RankBMFix> bm;
	bm.n_samples = 3;
	bm.add("rank25", rankbm_rank25, 15);
	bm.add("rank6", rankbm_rank6, 15);
	bm.add("rank3", rankbm_rank3, 15);
	bm.add("rankrr", rankbm_rankrr, 15);
	bm.run_all();
	bm.report(0);
}

