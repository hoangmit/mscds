#include "block_mem_mng.h"
#include "sdarray_block.h"

#include "intarray/sdarray.h"
#include "intarray/sdarray_sml.h"
#include "intarray/sdarray_th.h"
#include "intarray/sdarray_zero.h"
#include "fused_sdarray_test.h"
#include "sdarray_blk_hints.h"

#include "mem/info_archive.h"

#include "utils/benchmark.h"

using namespace mscds;

//--------------------------------------------------------------------------
// Benchmark

struct StmFix : public SharedFixtureItf {
	static const unsigned int SIZE  = 20000000;
	static const unsigned int QSIZE =  2000000;
	StmFix() { is_sort = false; lookup_query = true;}
	bool is_sort, lookup_query;

	void SetUp() {
		size = SIZE;
		// generate test cases and data structure here
		unsigned int range = 500;
		this->qsize = QSIZE;

		SDArrayBuilder bd1;
		SDArraySmlBuilder bd2;
		TwoSDA_Builder xd;
		SDArrayTHBuilder thb;
		SDRankSelectBuilderSml srsb;
		LiftStBuilder<SDArrayFuseBuilder> fsgbd;
		LiftStBuilder<SDArrayFuseHintsBuilder> fwhbd;
		zero.clear();
		xd.init();
		fsgbd.init();
		auto& rfsgbdx = fsgbd.g<0>();
		auto& rfwhbdx = fwhbd.g<0>();

		uint64_t sum =0;
		rfwhbdx.start_model();
		for (unsigned i = 0; i < size; ++i) {
			unsigned val = 1 + rand() % range;
			vals.push_back(val);
			rfwhbdx.model_add(val);
		}
		rfwhbdx.build_model();
		fwhbd.init();
		for (unsigned i = 0; i < size; ++i) {
			unsigned val = vals[i];
			bd1.add(val);
			bd2.add(val);
			xd.add(val, val);
			thb.add(val);
			zero.add(val);
			srsb.add(val);
			rfsgbdx.add(val);
			fsgbd.check_end_block();

			rfwhbdx.add(val);
			fwhbd.check_end_block();
			sum += val;
		}
		bd1.build(&sd1);
		bd2.build(&sd2);
		xd.build(&qs);
		thb.build(&th);
		srsb.build(&lkz);
		fsgbd.check_end_data();
		fsgbd.build(&fuse_single);
		fwhbd.check_end_data();
		fwhbd.build(&fuse_hints);

		this->size = size;
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
		if (lookup_query) {
			if (!is_sort)
				std::random_shuffle(lookupqs.begin(), lookupqs.end());
			queries = &lookupqs;
			return ;
		}
		for (unsigned int i = 0; i < qsize; ++i)
			rankqs.push_back((((unsigned)(rand() % 256) << 8) | (rand() % 256)) % sum);
		if (is_sort) {
			std::sort(rankqs.begin(), rankqs.end());
		}
		if (!lookup_query) {
			queries = &rankqs;
		}
	}

	void TearDown() { vals.clear(); sd1.clear(); sd2.clear(); qs.clear(); th.clear(); zero.clear(); lkz.clear(); fuse_single.clear(); }

	unsigned size, qsize;
	std::vector<unsigned> vals;
	std::vector<unsigned int> * queries;
	std::vector<unsigned int> lookupqs;
	std::vector<unsigned int> rankqs;
	SDArrayQuery sd1;
	SDArraySml sd2;
	TwoSDA_Query qs;
	SDArrayTH th;

	SDArrayZero zero;
	SDRankSelectSml lkz;

	LiftStQuery<SDArrayFuse> fuse_single;
	LiftStQuery<SDArrayFuseHints> fuse_hints;

	void report_size() {
		SetUp();
		std::cout << "> Data structure sizes (bytes)" << std::endl;
		std::cout << "vector" << "\t" << zero.cums.size() * sizeof(zero.cums[0]) << std::endl;
		std::cout << "sda_b64" << "\t" << estimate_data_size(sd1) << std::endl;
		std::cout << "sda_b512" << "\t" << estimate_data_size(sd2) << std::endl;
		std::cout << "sda_hints" << "\t" << estimate_data_size(lkz) << std::endl;
		std::cout << "sda_th" << "\t" << estimate_data_size(th) << std::endl;

		std::cout << "sda_fusion(2)" << "\t" << estimate_data_size(qs.mng) << std::endl;
		std::cout << "sda_one_fuse" << "\t" << estimate_data_size(fuse_single) << std::endl;
		std::cout << std::endl;
		std::cout << "> Components sizes (optional)" << std::endl;
		sd2.inspect("comp_size", std::cout);
		th.inspect("comp_size", std::cout);
		TearDown();
	}
};



//-------------------------------------------------
void sda_null(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.lookup((*qs)[i]);
	}
}

void sda_64(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.lookup((*qs)[i]);
	}
}

void sda_512(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.lookup((*qs)[i]);
	}
}

void sda_fuse0(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.lookup((*qs)[i]);
	}
}

void sda_fuse1(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.lookup((*qs)[i]);
	}
}

void sda_fusesingle(StmFix * fix) {
	auto& x = fix->fuse_single.g<0>();
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup((*qs)[i]);
	}
}

void sda_fuses_hints(StmFix * fix) {
	auto& x = fix->fuse_hints.g<0>();
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.lookup((*qs)[i]);
	}
}

void sda_th(StmFix * fix) {
	unsigned vx = 121;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.lookup((*qs)[i]);
	}
}

//-------------------------------------------------
void sda_null_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->zero.rank((*qs)[i]);
	}
}

void sda_64_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd1.rank((*qs)[i]);
	}
}

void sda_512_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->sd2.rank((*qs)[i]);
	}
}

void sda_fuse0_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.x.rank((*qs)[i]);
	}
}

void sda_fuse1_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->qs.y.rank((*qs)[i]);
	}
}

void sda_fusesingle_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	auto& x = fix->fuse_single.g<0>();
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.rank((*qs)[i]);
	}
}

void sda_fuses_hints_rank(StmFix * fix) {
	auto& x = fix->fuse_hints.g<0>();
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= x.rank((*qs)[i]);
	}
}


void sda_th_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->th.rank((*qs)[i]);
	}
}

void sda_hints_rank(StmFix * fix) {
	unsigned vx = 131;
	std::vector<unsigned int> * qs = fix->queries;
	for (unsigned i = 0; i < fix->qsize; ++i) {
		vx ^= fix->lkz.rank((*qs)[i]);
	}
}
//------------------------------------------------


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
	bm.add_remark("input_length: " + utils::tostr(n) +
		"\t query_set_length: " + utils::tostr(m));
	//bm.add("vector", sda_null_rnd, 100);
	bm.add("sda_b64_rnd_access", sda_64, 15);
	bm.add("sda_b512_rnd_access", sda_512, 15);
	bm.add("sda_th_rnd_access", sda_fuse1, 15);

	bm.add("sda_fuse0_rnd_access", sda_fuse0, 15);
	bm.add("sda_fuse1_rnd_access", sda_fuse1, 15);
	bm.add("sda_one_fuse_rnd_access", sda_fusesingle, 15);
	bm.add("sda_fuse_hints_rnd_rank", sda_fuses_hints, 15);
	StmFix f;
	f.is_sort = false; f.lookup_query = true;
	bm.run_all(&f);
	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_seq_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input_length: " + utils::tostr(n) +
		"\t query_set_length: " + utils::tostr(m));
	//bm.add("vector", sda_null, 100);
	bm.add("sda_b64_seq_access", sda_64, 15);
	bm.add("sda_b512_seq_access", sda_512, 15);
	bm.add("sda_th_seq_access", sda_th, 15);

	bm.add("sda_fuse0_seq_access", sda_fuse0, 15);
	bm.add("sda_fuse1_seq_access", sda_fuse1, 15);
	bm.add("sda_one_fuse_seq_access", sda_fusesingle, 15);
	bm.add("sda_fuse_hints_rnd_rank", sda_fuses_hints, 15);
	StmFix f;
	f.is_sort = true; f.lookup_query = true;
	bm.run_all(&f);

	bm.report(0); // <-- baseline
}

BENCHMARK_SET(sdarray_rnd_rank_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input_length: " + utils::tostr(n) +
		"\t query_set_length: " + utils::tostr(m));
	bm.add("vector", sda_null_rank, 15);
	bm.add("sda_b64_rnd_rank", sda_64_rank, 15);
	bm.add("sda_b512_rnd_rank", sda_512_rank, 15);
	bm.add("sda_b512hints_rnd_rank", sda_hints_rank, 15);
	bm.add("sda_th_rnd_rank", sda_th_rank, 15);

	bm.add("sda_fuse0_rnd_rank", sda_fuse0_rank, 15);
	bm.add("sda_fuse1_rnd_rank", sda_fuse1_rank, 15);
	bm.add("sda_one_fuse_rnd_rank", sda_fusesingle_rank, 15);
	bm.add("sda_fuse_hints_rnd_rank", sda_fuses_hints_rank, 15);

	StmFix f;
	f.is_sort = false; f.lookup_query = false;
	bm.run_all(&f);
	bm.report(0); // <-- baseline
}


BENCHMARK_SET(sdarray_seq_rank_benchmark) {
	srand(RANDSEED);
	Benchmarker<StmFix> bm;
	bm.n_samples = 3;
	auto n = StmFix::SIZE, m = StmFix::QSIZE;
	bm.add_remark("input_length: " + utils::tostr(n) +
		"\t query_set_length: " + utils::tostr(m));
	bm.add("vector", sda_null_rank, 15);
	bm.add("sda_b64_seq_rank", sda_64_rank, 15);
	bm.add("sda_b512_seq_rank", sda_512_rank, 15);
	bm.add("sda_b512hints_seq_rank", sda_hints_rank, 15);
	bm.add("sda_th_seq_rank", sda_th_rank, 15);

	bm.add("sda_fuse0_seq_rank", sda_fuse0_rank, 15);
	bm.add("sda_fuse1_seq_rank", sda_fuse1_rank, 15);
	bm.add("sda_one_fuse_seq_rank", sda_fusesingle_rank, 15);
	bm.add("sda_fuse_hints_rnd_rank", sda_fuses_hints_rank, 15);

	StmFix f;
	f.is_sort = true; f.lookup_query = false;
	bm.run_all(&f);
	bm.report(0); // <-- baseline
}
