
#include "intv/nintv.h"
#include "rlsum_trivbin.h"
#include "mem/file_archive2.h"
#include "utils/file_utils.h"
#include "utils/utest.h"
#include "RLSum6.h"

#include "intv/intv_rand_gen.hpp"
#include "cwig2/fused_intval2.h"


#include <vector>
#include <sstream>

using namespace std;
using namespace mscds;
using namespace app_ds;

std::deque<ValRange> convertVR(const std::deque<ValRangeInfo>& inp) {
	std::deque<ValRange> out(inp.size());
	for (unsigned i = 0; i < out.size(); ++i) {
		const ValRangeInfo& v = inp[i];
		out[i].st = v.st;
		out[i].ed = v.ed;
		out[i].val = v.val;
	}
	return out;
}

template<typename StructTp, bool tofile>
void test_rlsum_basic_vec(const vector<int>& A) {
	unsigned int len = A.size();
	std::deque<ValRange> inp = convertVR(genInp(A));
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
		OFileArchive2 fo;
		fo.open_write(fn);
		bd.build(fo);
		fo.close();

		IFileArchive2 fi;
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

//------------------------------------------------------------------------
template<typename RunLenSumType>
void test_rlsum_tb_rng(int test_num) {
	int len = 10000;
	vector<int> A = gen_density(len);
	test_rlsum_basic_vec<RunLenSumType, true>(A);
}

template<typename RunLenSumArray>
void test_rlsum_tb_1() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(0, 3, 1));
	inp.push_back(ValRange(5, 7, 9));
	inp.push_back(ValRange(7, 10, 2));
	inp.push_back(ValRange(10, 11, 3));*/

	const int len = 11;
	int A[len] = { 1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3 };

	vector<int> Av(A, A + len);
	test_rlsum_basic_vec<RunLenSumArray, false>(Av);
}

template<typename RunLenSumArray>
void test_rlsum_tb_2() {
	/*std::deque<ValRange> inp;
	inp.push_back(ValRange(3, 5, 2));
	inp.push_back(ValRange(6, 8, 3));
	inp.push_back(ValRange(9, 10, 4));
	inp.push_back(ValRange(10, 11, 5));*/

	const int len = 14;
	int A[len] = { 0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0 };

	vector<int> Av(A, A + len);
	test_rlsum_basic_vec<RunLenSumArray, true>(Av);
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

TEST(rlsum, fuse_1) {
	test_rlsum_tb_1<IntValQuery2>();
}

TEST(rlsum, fuse_2) {
	test_rlsum_tb_2<IntValQuery2>();
}

TEST(rlsum, fuse_rng) {
	for (int i = 0; i < 100; i++) {
		test_rlsum_tb_rng<IntValQuery2>(i);
		cout << ".";
	}
	cout << endl;
}
