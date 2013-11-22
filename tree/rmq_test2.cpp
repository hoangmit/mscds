
#include "RMQ_pm1.h"
#include "RMQ_index_table.h"
#include "utils/utils.h"
#include "utils/utest.h"
#include "utils/str_utils.h"
#include "RMQ_sct.h"
#include <iostream>
#include <cassert>

#include "mem/filearchive.h"

#include "celero/Celero.h"

using namespace std;
using namespace mscds;
using namespace utils;

void gen_excess_seq(const string& s) {
	int v = 0;
	int i = 1;
	for (char c : s) {
		if (c == ' ') continue;
		if (c == '0') v -= 1;
		else
		if (c == '1') v += 1;
		cout << v << ", ";
		if (i % 8 == 0) cout << endl;
		++i;
	}
	cout << endl;
}

void testmax(uint64_t v) {
	auto x = max_excess_word_slow(v);
	auto y = max_excess_word(v);
	if (x != y) {
		cout << "wrong: " << endl;
		string s = binstr2(v);
		cout << s << endl;
		cout << "expected: (" << (int)x.first << "," << (int)x.second << ")" << endl;
		cout << "actual  : (" << (int)y.first << "," << (int)y.second << ")" << endl;
		gen_excess_seq(s);
		max_excess_word(v);
		ASSERT_EQ(x, y);
	}
}

void testmin(uint64_t v) {
	auto x = min_excess_word_slow(v);
	auto y = min_excess_word(v);
	if (x != y) {
		cout << "wrong: " << endl;
		string s = binstr2(v);
		cout << s << endl;
		cout << "expected: (" << (int)x.first << "," << (int)x.second << ")" << endl;
		cout << "actual  : (" << (int)y.first << "," << (int)y.second << ")" << endl;
		gen_excess_seq(s);
		min_excess_word(v);
		ASSERT_EQ(x,y);
	}
}

TEST(rmq_pm1, min_max_excess_word) {
	for (unsigned int i = 0; i < 100000; ++i) {
		uint64_t v = rand64();
		testmax(v);
		testmin(v);
	}
}

void test_rmq2(unsigned int len, unsigned int range, bool min_val = true) {
	std::vector<int> vals = rand_vec<int>(len, range);
	RMQ_index_table tbl;
	RMQ_index_table::build(vals, min_val, &tbl);
	
	for (unsigned int i = 0; i < vals.size(); ++i) {
		for (unsigned int j = i + 1; j <= vals.size(); ++j) {
			size_t exp = RMQ_index_table::_slow_m_idx<int>(i, j, vals, min_val);
			auto ret = tbl.m_idx(i, j);
			ASSERT(ret.first <= ret.second);
			if (ret.first != exp && ret.second != exp) {
				ret = tbl.m_idx(i, j);
				ASSERT(ret.first == exp || ret.second == exp);
			}
		}
	}
}

TEST(rmq_pm1, min_max_struct1) {
	test_rmq2(10, 5);
	test_rmq2(10, 5);
	test_rmq2(16, 5);
	for (unsigned int i = 0; i < 10; ++i)
		test_rmq2(10, 5);
	for (unsigned int i = 0; i < 50; ++i)
		test_rmq2(30 + rand() % 4, 8 + rand() % 4, rand() % 2 == 0);
}

void test_idx_table(unsigned int len, unsigned int range, unsigned int blksize = 8, bool min_val = true) {
	std::vector<int> vals = rand_vec<int>(len, range);

	RMQ_index_blk tbl;
	RMQ_index_blk::build(vals, blksize, min_val, &tbl);

	for (unsigned int i = 0; i < vals.size(); ++i) {
		for (unsigned int j = i + 1; j <= vals.size(); ++j) {
			size_t exp = RMQ_index_table::_slow_m_idx<int>(i, j, vals, min_val);
			auto ret = tbl.m_idx(i, j);
			for (size_t k = 1; k < ret.size(); ++k) ASSERT(ret[k] > ret[k - 1]);
			bool found = false;
			for (size_t k = 0; k < ret.size(); ++k)
				if (exp == ret[k]) { found = true; break; }
			if (!found) {
				cout << "expected = " << exp << endl;
				cout << "result = {";
				for (size_t k = 0; k + 1 < ret.size(); ++k)
					cout << ret[k] << ", ";
				if (ret.size() > 0) cout << ret.back();
				cout << "}" << endl;
				ret = tbl.m_idx(i, j);
				ASSERT(found);
			}
		}
	}
}

TEST(rmq_pm1, blk_table) {
	test_idx_table(25, 5, 8, true);
	test_idx_table(100, 8, 8, true);

	test_idx_table(12, 10, 4, true);
	test_idx_table(32, 5, 4, true);
	test_idx_table(32, 5, 8, true);
	test_idx_table(64, 5, 8, true);
	for (unsigned int i = 0; i < 50; ++i) {
		SCOPED_TRACE("loop i = " + utils::tostr(i));
		test_idx_table(120 + rand() % 16, 8 + rand() % 4, 8, rand() % 2 == 0);
	}
}

void test_rmq_pm1(unsigned int len, unsigned blksize = 4, bool min_struct=true) {
	vector<int> vals(len);
	BitArray b = BitArray::create(len);
	int last = 0;
	vector<bool> bv = rand_bitvec(len);
	for (int i = 0; i < bv.size(); ++i) {
		bool bx = bv[i];
		b.setbit(i, bx);
		if (bx) last += 1;
		else last -= 1;
		vals[i] = last;
	}
	/*{
		unsigned int ll = utils::ceildiv(len, 64);
		vector<int> blkexp(ll);
		for (unsigned int i = 0; i < ll; ++i) {
			unsigned int edx = std::min((i + 1) * 64, len);
			blkexp[i] = *std::min_element(vals.begin() + i * 64, vals.begin() + edx);
		}
		for (unsigned int i = 0; i < ll; ++i)
			cout << blkexp[i] << ", ";
		cout << endl;
	}*/

	RMQ_index_table tbl;
	RMQ_index_table::build(vals, min_struct, &tbl);

	RMQ_pm1 rmq;
	RMQ_pm1::build(b, blksize, min_struct, &(rmq));
	for (unsigned int i = 0; i < vals.size(); ++i) {
		for (unsigned int j = i + 1; j <= vals.size(); ++j) {
			auto p = tbl.m_idx(i, j);
			size_t exp = p.first;
			if (min_struct) {
				if (vals[p.first] > vals[p.second]) exp = p.second;
			} else
				if (vals[p.first] < vals[p.second]) exp = p.second;
			auto ac = rmq.m_idx(i, j);
			if (exp != ac) {
				auto exp2 = RMQ_index_table::_slow_m_idx<int>(i, j, vals, min_struct);
				cout << "i = " << i << "     j = " << j << endl;
				cout << "expected = " << exp << "    (" << exp2 << ")"<< endl;
				cout << "actual   = " << ac << endl;
				rmq.m_idx(i, j);
				ASSERT_EQ(exp, ac);
			}
		}
	}
}

TEST(rmq_pm1, rmq_pm1) {
	test_rmq_pm1(769, 4, true);
	test_rmq_pm1(768, 4, true);
	for (unsigned int i = 0; i < 50; ++i) {
		SCOPED_TRACE("loop i = " + utils::tostr(i));
		cout << ".";
		if (i % 5 == 0) cout << flush;
		test_rmq_pm1(764 + rand() % 16, 4, rand() % 2 == 0);
	}
	cout << endl;
}

TEST(rmq_pm1, saveload) {
	unsigned int len = 10000, blksize = 8;
	bool min_struct = true;
	vector<bool> bv = rand_bitvec(len);
	vector<int> vals(len);
	BitArray b = BitArray::create(len);
	int last = 0;
	for (int i = 0; i < bv.size(); ++i) {
		bool bx = bv[i];
		b.setbit(i, bx);
		if (bx) last += 1;
		else last -= 1;
		vals[i] = last;
	}

	RMQ_pm1 rmq;
	RMQ_pm1::build(b, blksize, min_struct, &(rmq));
	OMemArchive out;
	rmq.save_aux(out);
	out.close();

	RMQ_pm1 nrmq;
	IMemArchive inx(out);
	nrmq.load_aux(inx, b);
	for (unsigned int i = 0; i < vals.size(); ++i) {
		unsigned int st = i;
		unsigned int ed = rand() % (vals.size() + 1);
		if (st > ed) std::swap(st, ed);
		auto x1 = nrmq.m_idx(st, ed);
		auto x2 = rmq.m_idx(st, ed);
		ASSERT_EQ(x2, x1);
	}
	inx.close();
}

#include "utils/str_utils.h"

void report_size(unsigned int len, unsigned int blksize, bool progress = false) {
	if (progress) cout << "Generating input ..." << endl;
	vector<bool> bv = rand_bitvec(len);
	vector<int> vals(len);
	BitArray b = BitArray::create(len);
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

class QueryFixture : public celero::TestFixture {
public:
	QueryFixture() {
		ProblemSetValues.push_back(1);
		unsigned int blksize = 32;
		unsigned int len = 2000000;
		unsigned int querycnt = 1000;
		vector<bool> bv = rand_bitvec(len);
		b = BitArray::create(len);
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
	virtual void SetUp(const int32_t problemSetValue) {
	}

	virtual void TearDown() {
		/*b.clear();
		vals.clear();
		queries.clear();

		rmq.clear();
		tblsim.clear();
		tblblk.clear();
		sct.clear();*/
	}

	BitArray b;
	std::vector<int> vals;
	std::vector<std::pair<unsigned int, unsigned int> > queries;
	RMQ_pm1 rmq;
	RMQ_index_table tblsim;
	RMQ_index_blk tblblk;
	RMQ_sct sct;
};

BASELINE_F(RMQ_pm1, table_big, QueryFixture, 5, 5) {
	for (auto p : queries) {
		celero::DoNotOptimizeAway(
			this->tblsim.m_idx(p.first, p.second));
	}
}

BENCHMARK_F(RMQ_pm1, table_smaller, QueryFixture, 5, 5) {
	for (auto p : queries) {
		celero::DoNotOptimizeAway(
			this->tblblk.m_idx(p.first, p.second));
	}
}


BENCHMARK_F(RMQ_pm1, rmq1, QueryFixture, 5, 5) {
	for (auto p : queries) {
		celero::DoNotOptimizeAway(
			this->rmq.m_idx(p.first, p.second));
	}
}


BENCHMARK_F(RMQ_pm1, sct, QueryFixture, 5, 5) {
	for (auto p : queries) {
		celero::DoNotOptimizeAway(
			this->sct.m_idx(p.first, p.second));
	}
}

int main(int argc, char* argv[]) {
	locale oldLoc = cout.imbue(locale(cout.getloc(), new comma_numpunct()));
	//test_size();
	celero::Run(argc, argv);
	return 0;

	//::testing::GTEST_FLAG(filter) = "rmq_pm1.rmq_pm1";
	::testing::InitGoogleTest(&argc, argv);
	int rs = RUN_ALL_TESTS();
	return rs;

	//print_max_excess_8_table();
}
