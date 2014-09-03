

#include "RMQ_pm1.h"
#include "RMQ_sct.h"
#include "RMQ_table.h"

#include "utils/benchmark.h"
#include "bitarray/bitarray.h"

#include "utils/utest.h"
#include "utils/str_utils.h"
#include "mem/info_archive.h"

#include <iostream>
#include <vector>

namespace tests {

using namespace std;
using namespace mscds;


void report_size(unsigned int len, unsigned int blksize, bool progress = false) {
	if (progress) cout << "Generating input ..." << endl;
	vector<bool> bv = rand_bitvec(len);
	vector<int> vals(len);
	BitArray b = BitArrayBuilder::create(len);
	int last = 0;
	for (int i = 0; i < bv.size(); ++i) {
		bool bx = bv[i];
		b.setbit(i, bx);
		if (bx) last += 1;
		else last -= 1;
		vals[i] = last;
	}
	bv.clear();


	RMQ_pm1 rmq;
	RMQ_index_table tblsim;
	RMQ_index_blk tblblk;


	RMQ_pm1::build(b, blksize, true, &(rmq));
	OSizeEstArchive ar;
	if (progress) cout << "Measuring size ..." << endl;

	cout << "Input length = " << len << " bits" << endl;
	cout << "Block size = " << blksize << endl;
	cout << endl;
	RMQ_pm1::build(b, blksize, true, &(rmq));
	rmq.save_aux(ar);
	size_t last_pos = 0, sz = 0;
	sz = ar.opos() - last_pos;
	last_pos = ar.opos();
	cout << "RMQ_pm1:" << endl;
	cout << "Aux Size = " << sz << " bytes" << endl;
	cout << "Ratio = " << ((sz * 8) / ((double)len)) * 100 << " %" << endl;
	cout << endl;

	RMQ_index_table::build(vals, true, &tblsim);
	tblsim.save(ar);
	sz = ar.opos() - last_pos;
	last_pos = ar.opos();
	cout << "RMQ_index_table:" << endl;
	cout << "Aux Size = " << sz << " bytes" << endl;
	cout << "Ratio = " << ((sz * 8) / ((double)len)) * 100 << " %" << endl;
	cout << endl;

	RMQ_index_blk::build(vals, blksize, true, &tblblk);
	tblblk.save(ar);
	sz = ar.opos() - last_pos;
	last_pos = ar.opos();
	cout << "RMQ_index_block:" << endl;
	cout << "Aux Size = " << sz << " bytes" << endl;
	cout << "Ratio = " << ((sz * 8) / ((double)len)) * 100 << " %" << endl;
	cout << endl;
}


void test_size() {
	unsigned int len = 10000000;

	cout << "RMQ_pm1 auxiliary size measurement" << endl;

	//report_size(len, 16);
	report_size(len, 32);
	//report_size(len, 64);
}

class RMQQuerySFixture {
public:
	RMQQuerySFixture() {}

	void SetUp() {
		unsigned int blksize = 32;
		unsigned int len = 10000000;
		unsigned int querycnt = 10000;
		vector<bool> bv = rand_bitvec(len);
		b = BitArrayBuilder::create(len);
		vals.resize(len);
		int last = 0;
		for (int i = 0; i < bv.size(); ++i) {
			bool bx = bv[i];
			this->b.setbit(i, bx);
			if (bx) last += 1;
			else last -= 1;
			this->vals[i] = last;
		}
		RMQ_pm1::build(b, blksize, true, &(rmq));
		RMQ_index_table::build(vals, true, &tblsim);
		RMQ_index_blk::build(vals, blksize, true, &tblblk);
		sct.build(vals, true);
		queries.clear();
		for (unsigned int i = 0; i < querycnt; ++i) {
			unsigned int st = rand() % len;
			unsigned int ed = rand() % (len + 1);
			if (st > ed) std::swap(st, ed);
			queries.push_back(make_pair(st, ed));
		}
	}

	void TearDown() {
		b.clear();
		vals.clear();
		queries.clear();

		rmq.clear();
		tblsim.clear();
		tblblk.clear();
		sct.clear();
	}

	BitArray b;
	std::vector<int> vals;
	std::vector<std::pair<unsigned int, unsigned int> > queries;
	RMQ_pm1 rmq;
	RMQ_index_table tblsim;
	RMQ_index_blk tblblk;
	RMQ_sct sct;
};
/*
template<class T> void DoNotOptimizeAway(T&& datum)
{
#ifdef WIN32
if (_getpid() == 1)
#else
if (getpid() == 1)
#endif
{
const void* p = &datum;
putchar(*static_cast<const char*>(p));
}
}*/

#define DoNotOptimizeAway(T) (T)

void RMQ_pm1_table_big(RMQQuerySFixture * fixture) {
	for (auto p : fixture->queries) {
		DoNotOptimizeAway(fixture->tblsim.m_idx(p.first, p.second));
	}
}

void RMQ_pm1_table_smaller(RMQQuerySFixture * fixture) {
	for (auto p : fixture->queries) {
		fixture->tblblk.m_idx(p.first, p.second);
	}
}

void RMQ_pm1_rmq1(RMQQuerySFixture* fixture) {
	for (auto p : fixture->queries) {
		fixture->rmq.m_idx(p.first, p.second);
	}
}

void RMQ_pm1_sct(RMQQuerySFixture * fixture) {
	for (auto p : fixture->queries) {
		fixture->sct.m_idx(p.first, p.second);
	}
}

BENCHMARK_SET(rmq_benchmark) {
	Benchmarker<RMQQuerySFixture> bm;
	bm.n_samples = 3;

	bm.add("RMQ_pm1_table_big", RMQ_pm1_table_big, 50);
	bm.add("RMQ_pm1_table_smaller", RMQ_pm1_table_smaller, 10);
	bm.add("RMQ_pm1_rmq1", RMQ_pm1_rmq1, 10);
	bm.add("RMQ_pm1_sct", RMQ_pm1_sct, 10);

	bm.run_all();
	bm.report(0);
}

}//namespace