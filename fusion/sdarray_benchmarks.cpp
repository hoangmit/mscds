#include "blkgroup_array.h"
#include "sdarray_block.h"

#include "intarray/sdarray.h"
#include "intarray/sdarray_sml.h"
#include "intarray/sdarray_th.h"
#include "intarray/sdarray_zero.h"
#include "fused_sdarray_test.h"

#include "mem/info_archive.h"

#include "utils/benchmark.h"

using namespace mscds;

//--------------------------------------------------------------------------
// Benchmark

struct StmFix : public SharedFixtureItf {
	static const unsigned int SIZE  = 20000000;
	static const unsigned int QSIZE =  2000000;
	void SetUp(int size) {
		if (size <= 0) { size = SIZE; }
		// generate test cases and data structure here
		unsigned int range = 500;
		this->qsize = QSIZE;

		SDArrayBuilder bd1;
		SDArraySmlBuilder bd2;
		TwoSDA_Builder xd;
		SDArrayTHBuilder thb;
		SDRankSelectBuilderSml srsb;
		LiftStBuilder<SDArrayFuseBuilder> fsgbd;
		zero.clear();
		xd.init();
		fsgbd.init();
		auto& rfsgbdx = fsgbd.g<0>();

		uint64_t sum =0;
		
		for (unsigned i = 0; i < size; ++i) {
			unsigned val = 1 + rand() % range;
			vals.push_back(val);
			bd1.add(val);
			bd2.add(val);
			xd.add(val, val);
			thb.add(val);
			zero.add(val);
			srsb.add(val);
			rfsgbdx.add(val);
			if (fsgbd.is_all_full()) {
				fsgbd._end_block();
			}
			sum += val;
		}
		bd1.build(&sd1);
		bd2.build(&sd2);
		xd.build(&qs);
		thb.build(&th);
		srsb.build(&lkz);
		if (!fsgbd.is_all_empty())
			fsgbd._end_block();
		fsgbd.build(&fuse_single);
		this->size = size;
		queries.resize(qsize);
		assert(qsize <= size);
		if (qsize == size) {
			for (unsigned i = 0; i < qsize; ++i)
				queries[i] = i;
		} else {
			unsigned d = size  / qsize;
			for (unsigned i = 0; i < qsize - 1; ++i)
				queries[i] = (unsigned)(d*i);
		}
		std::random_shuffle(queries.begin(), queries.end());
		for (unsigned int i = 0; i < qsize; ++i)
			rankqs.push_back((((unsigned)(rand() % 256) << 8) | (rand() % 256)) % sum);
		rankseq = rankqs;
		std::sort(rankseq.begin(), rankseq.end());
	}

	void TearDown() {vals.clear(); sd1.clear(); sd2.clear(); qs.clear(); th.clear(); zero.clear(); lkz.clear(); }

	unsigned size, qsize;
	std::vector<unsigned> vals;
	std::vector<unsigned int> queries;
	std::vector<unsigned int> rankqs, rankseq;
	SDArrayQuery sd1;
	SDArraySml sd2;
	TwoSDA_Query qs;
	SDArrayTH th;

	SDArrayZero zero;
	SDRankSelectSml lkz;

	LiftStQuery<SDArrayFuse> fuse_single;

	void report_size() {
		SetUp(0);
		std::cout << "Data structure sizes (bytes):" << std::endl;
		std::cout << "sda_b64" << "\t" << estimate_data_size(sd1) << std::endl;
		std::cout << "sda_b512" << "\t" << estimate_data_size(sd2) << std::endl;
		std::cout << "sda_fusion(2)" << "\t" << estimate_data_size(qs.mng) << std::endl;
		std::cout << "sda_one_fuse" << "\t" << estimate_data_size(fuse_single) << std::endl;
		std::cout << "sda_th" << "\t" << estimate_data_size(th) << std::endl;
		std::cout << "sda_hints" << "\t" << estimate_data_size(lkz) << std::endl;
		std::cout << "vector" << "\t" << zero.cums.size() * sizeof(zero.cums[0]) << std::endl;
		std::cout << std::endl;
		std::cout << "Components sizes (optional):" << std::endl;
		sd2.inspect("comp_size", std::cout);
		th.inspect("comp_size", std::cout);
		TearDown();
	}
};


void sda_null(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.lookup(i*d);
	}
}

void sda_64(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.lookup(i*d);
	}
}

void sda_512(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.lookup(i*d);
	}
}

void sda_fuse0(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.lookup(i*d);
	}
}

void sda_fuse1(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.lookup(i*d);
	}
}

void sda_fusesingle(StmFix * fix) {
	unsigned vx = 101;
	auto& x = fix->fuse_single.g<0>();
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup(i*d);
	}
}

void sda_th(StmFix * fix) {
	unsigned vx = 101;
	unsigned d = fix->size  / fix->qsize;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.lookup(i*d);
	}
}

//-------------------------------------------------
void sda_null_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.lookup(fix->queries[i]);
	}
}

void sda_64_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.lookup(fix->queries[i]);
	}
}

void sda_512_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.lookup(fix->queries[i]);
	}
}

void sda_fuse0_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.lookup(fix->queries[i]);
	}
}

void sda_fuse1_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.lookup(fix->queries[i]);
	}
}

void sda_fusesingle_rnd(StmFix * fix) {
	auto& x = fix->fuse_single.g<0>();
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup(fix->queries[i]);
	}
}


void sda_th_rnd(StmFix * fix) {
	unsigned vx = 121;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.lookup(fix->queries[i]);
	}
}

//-------------------------------------------------
void sda_null_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.rank(fix->rankqs[i]);
	}
}

void sda_64_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.find(fix->rankqs[i]);
	}
}

void sda_512_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.rank(fix->rankqs[i]);
	}
}

void sda_fuse0_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.rank(fix->rankqs[i]);
	}
}

void sda_fuse1_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.rank(fix->rankqs[i]);
	}
}

void sda_fusesingle_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	auto& x = fix->fuse_single.g<0>();
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.rank(fix->rankqs[i]);
	}
}

void sda_th_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.rank(fix->rankqs[i]);
	}
}

void sda_hints_rnd_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->lkz.rank(fix->rankqs[i]);
	}
}
//------------------------------------------------

void sda_null_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.rank(fix->rankseq[i]);
	}
}

void sda_64_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.find(fix->rankseq[i]);
	}
}

void sda_512_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.rank(fix->rankseq[i]);
	}
}

void sda_fuse0_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.rank(fix->rankseq[i]);
	}
}

void sda_fuse1_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.rank(fix->rankseq[i]);
	}
}

void sda_fusesingle_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	auto& x = fix->fuse_single.g<0>();
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.rank(fix->rankseq[i]);
	}
}

void sda_th_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.rank(fix->rankseq[i]);
	}
}

void sda_hints_seq_rank(StmFix * fix) {
	unsigned vx = 131;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->lkz.rank(fix->rankseq[i]);
	}
}

//-------------------------------------------------

static const unsigned int RANDSEED = 3571;

BENCHMARK_SET(size_report) {
	srand(RANDSEED);
	StmFix x;
	x.report_size();
}


BENCHMARK_SET(sdarray_rnd_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input length: " + utils::tostr(n) +
		"  query set length: " + utils::tostr(m));
	//bm.add("vector", sda_null_rnd, 100);
	bm.add("sda_b64_rnd_access", sda_64_rnd, 15);
	bm.add("sda_b512_rnd_access", sda_512_rnd, 15);
	bm.add("sda_fuse0_rnd_access", sda_fuse0_rnd, 15);
	bm.add("sda_fuse1_rnd_access", sda_fuse1_rnd, 15);
	bm.add("sda_one_fuse_rnd_access", sda_fusesingle_rnd, 15);
	bm.add("sda_th_rnd_access", sda_fuse1_rnd, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_seq_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input length: " + utils::tostr(n) +
		"  query set length: " + utils::tostr(m));
	//bm.add("vector", sda_null, 100);
	bm.add("sda_b64_seq_access", sda_64, 15);
	bm.add("sda_b512_seq_access", sda_512, 15);
	bm.add("sda_fuse0_seq_access", sda_fuse0, 15);
	bm.add("sda_fuse1_seq_access", sda_fuse1, 15);
	bm.add("sda_one_fuse_seq_access", sda_fusesingle, 15);
	bm.add("sda_th_seq_access", sda_th, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_rnd_rank_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input length: " + utils::tostr(n) +
		"  query set length: " + utils::tostr(m));
	bm.add("vector", sda_null_rnd_rank, 15);
	bm.add("sda_b64_rnd_rank", sda_64_rnd_rank, 15);
	bm.add("sda_b512_rnd_rank", sda_512_rnd_rank, 15);
	bm.add("sda_fuse0_rnd_rank", sda_fuse0_rnd_rank, 15);
	bm.add("sda_fuse1_rnd_rank", sda_fuse1_rnd_rank, 15);
	bm.add("sda_one_fuse_rnd_rank", sda_fusesingle_rnd_rank, 15);
	bm.add("sda_th_rnd_rank", sda_th_rnd_rank, 15);
	bm.add("sda_b512hints_rnd_rank", sda_hints_rnd_rank, 15);
	
	bm.run_all();
	bm.report(0); // <-- baseline
}


BENCHMARK_SET(sdarray_seq_rank_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input length: " + utils::tostr(n) +
		"  query set length: " + utils::tostr(m));
	bm.add("vector", sda_null_rnd_rank, 15);
	bm.add("sda_b64_seq_rank", sda_64_seq_rank, 15);
	bm.add("sda_b512_seq_rank", sda_512_seq_rank, 15);
	bm.add("sda_fuse0_seq_rank", sda_fuse0_seq_rank, 15);
	bm.add("sda_fuse1_seq_rank", sda_fuse1_seq_rank, 15);
	bm.add("sda_one_fuse_seq_rank", sda_fusesingle_seq_rank, 15);
	bm.add("sda_th_seq_rank", sda_th_seq_rank, 15);
	bm.add("sda_b512hints_seq_rank", sda_hints_seq_rank, 15);

	bm.run_all();
	bm.report(0); // <-- baseline
}

