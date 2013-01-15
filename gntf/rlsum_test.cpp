
#include "RLSum.h"
#include "RLSum3.h"

#include "rlsum_trivbin.h"
#include "mem/filearchive.h"
#include "utils/file_utils.h"
#include "utils/utest.h"

#include <vector>
#include <sstream>

using namespace std;
using namespace mscds;
using namespace app_ds;

void test_tb_1() {
	const int len = 11;
	int A[len] = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	int S[len+1];
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		S[i] = S[i-1] + A[i-1];

	std::deque<ValRange> inp;
	inp.push_back(ValRange(0, 3, 1));
	inp.push_back(ValRange(5, 7, 9));
	inp.push_back(ValRange(7, 10, 2));
	inp.push_back(ValRange(10, 11, 3));

	TrivBin x;
	x.build(inp);

	RunLenSumArray y;
	RunLenSumArrayBuilder bd;
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	bd.build(&y);

	for (int i = 0; i < inp.size(); ++i) {
		ASSERT(x.start[i] == y.range_start(i));
		ASSERT(x.sum_[i] == y.range_psum(i));
		ASSERT(x.rlen[i] == y.range_len(i));
	}

	for (int i = 0; i < len; ++i) {
		ASSERT(S[i] == x.sum(i));
		ASSERT(S[i] == y.sum(i));
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
	cout << ".";
}


void test_tb_2() {
	const int len = 14;
	int A[len] = {0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0};
	int S[len+1];
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		S[i] = S[i-1] + A[i-1];

	std::deque<ValRange> inp;
	inp.push_back(ValRange(3, 5, 2));
	inp.push_back(ValRange(6, 8, 3));
	inp.push_back(ValRange(9, 10, 4));
	inp.push_back(ValRange(10, 11, 5));

	TrivBin x;
	x.build(inp);

	RunLenSumArrayBuilder bd;
	std::stringstream ss(std::stringstream::in|std::stringstream::out|std::stringstream::binary);
	OFileArchive fo;
	fo.assign_write(&ss);
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	bd.build(fo);
	ss.flush();
	RunLenSumArray y;
	IFileArchive fi;
	fi.assign_read(&ss);
	y.load(fi);

	for (int i = 0; i < inp.size(); ++i) {
		ASSERT(x.start[i] == y.range_start(i));
		ASSERT(x.sum_[i] == y.range_psum(i));
		ASSERT(x.rlen[i] == y.range_len(i));
	}

	for (int i = 0; i < len; ++i) {
		ASSERT(S[i] == x.sum(i));
		ASSERT(S[i] == y.sum(i));
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
	cout << ".";
}

void test_cmp_rlsum3() {
	const int len = 14;
	int A[len] = {0, 0, 0, 2, 2, 0, 3, 3, 0, 4, 5, 0, 0, 0};
	int S[len+1];
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		S[i] = S[i-1] + A[i-1];

	std::deque<ValRange> inp;
	inp.push_back(ValRange(3, 5, 2));
	inp.push_back(ValRange(6, 8, 3));
	inp.push_back(ValRange(9, 10, 4));
	inp.push_back(ValRange(10, 11, 5));

	TrivBin x;
	x.build(inp);

	RunLenSumArrayBuilder3 bd;
	std::stringstream ss(std::stringstream::in|std::stringstream::out|std::stringstream::binary);
	OFileArchive fo;
	fo.assign_write(&ss);
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	bd.build(fo);
	ss.flush();
	RunLenSumArray3 y;
	IFileArchive fi;
	fi.assign_read(&ss);
	y.load(fi);

	for (int i = 0; i < inp.size(); ++i) {
		ASSERT(x.start[i] == y.range_start(i));
		ASSERT(x.rlen[i] == y.range_len(i));
		ASSERT(x.sum_[i] == y.range_psum(i));
	}

	for (int i = 0; i < len; ++i) {
		ASSERT(S[i] == x.sum(i));
		ASSERT(S[i] == y.sum(i));
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



void test_tb_rng(int test_num) {
	int len = 10000;
	vector<int> A = generate(len);
	vector<int> S(len+1);
	S[0] = 0;
	for (int i = 1; i <= len; i++)
		S[i] = S[i-1] + A[i-1];
	int lastv = 0, start;
	std::deque<ValRange> inp;
	A.push_back(0);
	for (int i = 0; i < len+1; i++) {
		if (A[i] != lastv) {
			if (lastv != 0) 
				inp.push_back(ValRange(start, i, lastv));
			start = i;
		}
		lastv = A[i];
	}
	TrivBin x;
	x.build(inp);

	RunLenSumArrayBuilder bd;
	string fn = utils::get_temp_path() + "test_file2f";
	
	for (unsigned int i = 0; i < inp.size(); ++i)
		bd.add(inp[i].st, inp[i].ed, inp[i].val);
	OFileArchive fo;
	fo.open_write(fn);
	bd.build(fo);
	fo.close();

	RunLenSumArray y;
	IFileArchive fi;
	fi.open_read(fn);
	y.load(fi);
	fi.close();

	for (int i = 0; i < inp.size(); ++i) {
		ASSERT(x.start[i] == y.range_start(i));
		ASSERT(x.sum_[i] == y.range_psum(i));
		ASSERT(x.rlen[i] == y.range_len(i));
	}

	for (int i = 0; i < len; ++i) {
		ASSERT(S[i] == x.sum(i));
		ASSERT(S[i] == y.sum(i));
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
	int lastv = 0, start;
	std::deque<ValRange> inp;
	A.push_back(0);
	for (int i = 0; i < len+1; i++) {
		if (A[i] != lastv) {
			if (lastv != 0) 
				inp.push_back(ValRange(start, i, lastv));
			start = i;
		}
		lastv = A[i];
	}

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
	int lastv = 0, start;
	std::deque<ValRange> inp;
	A.push_back(0);
	for (int i = 0; i < len+1; i++) {
		if (A[i] != lastv) {
			if (lastv != 0) 
				inp.push_back(ValRange(start, i, lastv));
			start = i;
		}
		lastv = A[i];
	}

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
	exp = -1;
	for (int i = len - 1; i >= 0; --i) {
		if (A[i] != 0) {
			ASSERT_EQ(i, y.next(i));
			exp = i;
		} else ASSERT_EQ(exp, y.next(i));
	}
}


void test_trivbin_all() {
	test_cmp_rlsum3();

	test_tb_1();
	test_tb_2();
	test_sum_delta1();
	test_sum_delta2();
	for (int i = 0; i < 1000; i++) {
		test_tb_rng(i);
	}	
}


int main() {
	test_trivbin_all();
	cout << endl;
	return 0;
}
