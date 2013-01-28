
#include "RLSum.h"
#include "RLSum4.h"

#include "noint.h"
#include "rlsum_trivbin.h"
#include "mem/filearchive.h"
#include "utils/file_utils.h"
#include "utils/utest.h"

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
void test_basic_vec(const vector<int>& A) {
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
void test_intervals(const std::deque<ValRange>& rng) {
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
	}
	size_t mlen = rng.back().ed;
	size_t j = 0;
	size_t cnt = 0;
	for (size_t i = 0; i < mlen; ++i) {
		if (rng[j].ed <= i) ++j;
		auto v = r.find_interval(i);
		ASSERT_EQ(cnt, r.coverage(i));
		if (rng[j].st <= i) {
			++cnt;
			ASSERT_EQ(v.second, true);
		} else ASSERT_EQ(v.second, false);
		ASSERT_EQ(v.first, j);
	}
	ASSERT_EQ(j, rng.size() - 1);
	auto v = r.find_interval(mlen);
	ASSERT_EQ(v.first, rng.size());
	ASSERT_EQ(v.second, false);
}

void test_tb_1() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(0, 3, 1));
	inp.push_back(ValRange(5, 7, 9));
	inp.push_back(ValRange(7, 10, 2));
	inp.push_back(ValRange(10, 11, 3));*/

	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};

	vector<int> Av(A,A+len);
	test_basic_vec<RunLenSumArray, false>(Av);
	cout << ".";
}


void test_tb_2() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(3, 5, 2));
	inp.push_back(ValRange(6, 8, 3));
	inp.push_back(ValRange(9, 10, 4));
	inp.push_back(ValRange(10, 11, 5));*/

	const int len = 14;
	int A[len] = {0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0};

	vector<int> Av(A,A+len);
	test_basic_vec<RunLenSumArray, true>(Av);
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


void test_start_end1() {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	test_intervals<NOInt>(genInp(Av));
}

void test_start_end2() {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = generate(len);
		test_intervals<NOInt>(genInp(A));
		if (i % 10 == 0) cout << '.';
	}
}

void test_start_end3() {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	vector<int> Av(A,A+len);
	test_intervals<NOInt2>(genInp(Av));
}

void test_start_end4() {
	int len = 10000;
	for (size_t i = 0; i < 100; ++i) {
		vector<int> A = generate(len);
		test_intervals<NOInt2>(genInp(A));
		if (i % 10 == 0) cout << '.';
	}
}


//------------------------------------------------------------------------
void test_tb_rng(int test_num) {
	int len = 10000;
	vector<int> A = generate(len);
	test_basic_vec<RunLenSumArray, true>(A);
	cout << ".";
}

void test_sum_delta1() {
	unsigned int len = 2000;
	int delta = 1;

	vector<int> A = generate(len);
	vector<int> S(len+1);
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		if (A[i-1] != 0)
			S[i] = S[i-1] + A[i-1] + delta;
		else
			S[i] = S[i-1];
	std::deque<ValRange> inp = genInp(A);

	RunLenSumArrayBuilder bd;
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	RunLenSumArray y;
	bd.build(&y);
	for (int i = 0; i < len; ++i) {
		if (S[i] != y.sum_delta(i, delta)) {
			std::cout << S[i] << endl;
			ASSERT(S[i] == y.sum_delta(i, delta));
		}
	}
}

void test_sum_delta2() {
	unsigned int len = 2000;
	int delta = -1;

	vector<int> A = generate(len);
	vector<int> S(len+1);
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		if (A[i-1] != 0)
			S[i] = S[i-1] + A[i-1] + delta;
		else
			S[i] = S[i-1];
	std::deque<ValRange> inp = genInp(A);

	RunLenSumArrayBuilder bd;
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	RunLenSumArray y;
	bd.build(&y);
	for (int i = 0; i < len; ++i) {
		if (S[i] != y.sum_delta(i, delta)) {
			cout << S[i] << " " << y.sum_delta(i, delta) << endl;
			ASSERT(S[i] == y.sum_delta(i, delta));
		}
	}
}

void test_rls4() {
	int len = 2000;
	vector<int> A = generate(len);
	test_basic_vec<RunLenSumArray4, true>(A);
}

void test_trivbin_all() {
	test_start_end3();
	test_start_end4();

	test_start_end1();
	test_start_end2();

	test_tb_1();
	test_tb_2();
	test_rls4();
	for (int i = 0; i < 1000; i++) {
		test_tb_rng(i);
	}	

	test_sum_delta1();
	test_sum_delta2();
}


int main() {
	test_trivbin_all();
	cout << endl;
	return 0;
}
