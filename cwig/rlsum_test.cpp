

#include "nintv.h"
#include "rlsum_trivbin.h"
#include "mem/filearchive.h"
#include "utils/file_utils.h"
#include "utils/utest.h"
#include "RLSum6.h"

#include <vector>
#include <sstream>

using namespace std;
using namespace mscds;
using namespace app_ds;


std::deque<ValRange> genInp(const vector<int>& A) {
	std::deque<ValRange> inp;
	int len = A.size();
	int lastv = 0, start;
	for (int i = 0; i < len; i++) {
		if (A[i] != lastv) {
			if (lastv != 0) 
				inp.push_back(ValRange(start, i, lastv));
			start = i;
		}
		lastv = A[i];
	}
	if (lastv != 0)
		inp.push_back(ValRange(start, len, lastv));
	return inp;
}

template<typename StructTp, bool tofile>
void test_rlsum_basic_vec(const vector<int>& A) {
	unsigned int len = A.size();
	std::deque<ValRange> inp = genInp(A);
	vector<int> S(len+1);
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		S[i] = S[i-1] + A[i-1];

	typename StructTp::BuilderTp bd;
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	StructTp y;
	if (!tofile) {
		bd.build(&y);
	}else {
		string fn = utils::get_temp_path() + "test_file2f";
		OFileArchive fo;
		fo.open_write(fn);
		bd.build(fo);
		fo.close();

		IFileArchive fi;
		fi.open_read(fn);
		y.load(fi);
		fi.close();
	}

	TrivBin x;
	x.build(inp);
	for (int i = 0; i < inp.size(); ++i) {
		ASSERT_EQ(x.start[i], y.range_start(i));
		ASSERT_EQ(x.rlen[i], y.range_len(i));
		ASSERT_EQ(x.sum_[i], y.range_psum(i));

		ASSERT_EQ(inp[i].st, y.range_start(i));
		ASSERT_EQ(inp[i].ed - inp[i].st, y.range_len(i));
		ASSERT_EQ(inp[i].val, y.range_value(i));
		ASSERT_EQ(inp[i].val, (x.sum_[i+1]-x.sum_[i])/x.rlen[i]);
	}

	for (int i = 0; i < len; ++i) {
		ASSERT_EQ(S[i], x.sum(i));
		ASSERT_EQ(S[i], y.sum(i));
	}
	for (int i = 0; i < len; ++i) {
		ASSERT_EQ(A[i], y.access(i));
	}

	int exp =-1, nzc = 0;
	for (int i = 0; i < len; ++i) {
		ASSERT_EQ(nzc, y.countnz(i));
		if (A[i] != 0){
			ASSERT_EQ(i, y.prev(i));
			exp = i;
			nzc++;
		} else ASSERT_EQ(exp, y.prev(i));
	}
	exp =-1;
	for (int i = len - 1; i >= 0; --i) {
		if (A[i] != 0){
			ASSERT_EQ(i, y.next(i));
			exp = i;
		} else ASSERT_EQ(exp, y.next(i));
	}
}

template<typename StructTp>
void test_intervals(const std::deque<ValRange>& rng, int testid=0) {
	typename StructTp::BuilderTp bd;
	for (size_t i = 0; i < rng.size(); ++i)
		bd.add(rng[i].st, rng[i].ed);
	StructTp r;
	bd.build(&r);
	ASSERT_EQ(rng.size(), r.length());
	for (size_t i = 0; i < rng.size(); ++i) {
		ASSERT_EQ(rng[i].st, r.int_start(i));
		ASSERT_EQ(rng[i].ed, r.int_end(i));
		ASSERT_EQ(rng[i].ed - rng[i].st, r.int_len(i));
		auto px = r.int_startend(i);
		ASSERT(rng[i].st == px.first && rng[i].ed == px.second);
	}
	for (size_t i = 0; i < rng.size() - 3; ++i) {
		typename StructTp::Enum e;
		r.getEnum(i, &e);
		for (size_t j = 0; j < 3; ++j) {
			auto p = e.next();
			ASSERT_EQ(rng[i+j].st, p.first);
			ASSERT_EQ(rng[i+j].ed, p.second);
		}
	}
	size_t mlen = rng.back().ed;
	size_t j = 0, jp = r.npos();
	size_t cnt = 0;
	for (size_t i = 0; i < mlen; ++i) {
		if (rng[j].ed <= i) ++j;
		if (rng[j].st <= i) jp = j;
		auto v = r.rank_interval(i);
		ASSERT_EQ(jp, v);
		ASSERT_EQ(cnt, r.coverage(i)) << "test id = " << testid << "   i = " << i << endl;
		auto p = r.find_cover(i);
		if (rng[j].st <= i) {
			++cnt;
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(i - rng[j].st + 1, p.second);
		}else {
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(0, p.second);
		}
	}
	ASSERT_EQ(rng.size() - 1, j);
	auto v = r.rank_interval(mlen);
	ASSERT_EQ(rng.size() - 1, v);
}

template<typename RunLenSumArray>
void test_rlsum_tb_1() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(0, 3, 1));
	inp.push_back(ValRange(5, 7, 9));
	inp.push_back(ValRange(7, 10, 2));
	inp.push_back(ValRange(10, 11, 3));*/

	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};

	vector<int> Av(A,A+len);
	test_rlsum_basic_vec<RunLenSumArray, false>(Av);
	cout << ".";
}

template<typename RunLenSumArray>
void test_rlsum_tb_2() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(3, 5, 2));
	inp.push_back(ValRange(6, 8, 3));
	inp.push_back(ValRange(9, 10, 4));
	inp.push_back(ValRange(10, 11, 5));*/

	const int len = 14;
	int A[len] = {0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0};

	vector<int> Av(A,A+len);
	test_rlsum_basic_vec<RunLenSumArray, true>(Av);
	cout << ".";
}


vector<int> generate(int len) {
	int last = 0;
	const int VAL_RANGE = 5;
	vector<int> ret(len);
	for (int i = 0; i < len; i++) {
		if (last == 0) {
			if (rand() % 100 < 50) ret[i] = 0;
			else ret[i] = (rand() % VAL_RANGE) + 1;
		}else {
			if (rand() % 100 < 90) ret[i] = last;
			else {
				if (rand() % 100 < 40) ret[i] = (rand() % VAL_RANGE) + 1;
				else ret[i] = 0;
			}
		}
		last = ret[i];
	}
	return ret;
}


TEST(intv, basic1) {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	test_intervals<NIntv>(genInp(Av));
}

TEST(intv, basic2) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = generate(len);
		test_intervals<NIntv>(genInp(A));
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}

TEST(intv, group1) {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	auto iv = genInp(Av);
	test_intervals<NIntvGroup>(iv);
}

TEST(intv, group2) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = generate(len);
		auto iv = genInp(A);
		test_intervals<NIntvGroup>(iv, i);
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}


TEST(intv, gap1) {
	const int len = 11;
	int A[len] = { 1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3 };
	vector<int> Av(A, A + len);
	auto iv = genInp(Av);
	test_intervals<NIntvGap>(iv);
}

TEST(intv, gap2) {
	const int len = 14;
	int A[len] = { 0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0 };
	vector<int> Av(A, A + len);
	auto iv = genInp(Av);
	test_intervals<NIntvGap>(iv);
}


TEST(intv, gap3) {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = generate(len);
		auto iv = genInp(A);
		test_intervals<NIntvGap>(iv, i);
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}


//------------------------------------------------------------------------
template<typename RunLenSumType>
void test_rlsum_tb_rng(int test_num) {
	int len = 10000;
	vector<int> A = generate(len);
	test_rlsum_basic_vec<RunLenSumType, true>(A);
}


TEST(rlsum, tb_1) {
	test_rlsum_tb_1<RunLenSumArray6>();
}

TEST(rlsum, tb_2) {
	test_rlsum_tb_2<RunLenSumArray6>();
}

TEST(rlsum, tb_rng) {
	for (int i = 0; i < 100; i++) {
		test_rlsum_tb_rng<RunLenSumArray6>(i);
		cout << ".";
	}
	cout << endl;
}

