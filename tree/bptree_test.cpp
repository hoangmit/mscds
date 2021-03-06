#include "BP_bits.h"

#include <iomanip>
#include <stack>
#include <vector>
#include <cstdlib>
#include <functional>

#include "mem/file_archive2.h"
#include "mem/info_archive.h"

#include "utils/str_utils.h"
#include "utils/file_utils.h"
#include "RMQ_sct.h"

#include "utils/utest.h"
#include "utils/utils.h"

namespace tests {

using namespace std;
using namespace utils;
using namespace mscds;

template<typename Func>
void print_table8b(Func fnc) {
	cout << "{";
	for (int i = 0; i < 255; i++) {
		if (i > 0 && i % 16 == 0) cout << endl;
		cout << setw(2) << (int)fnc(i) << ',';
	}
	cout << setw(2) << (int)fnc(255) << '}' << endl;

}

void print_ex() {
	// BP_block::min_revex_8c
	// BP_block::excess_8c
	// BP_block::min_excess_8c
	// BP_block::min_ex_pos_8c
	print_table8b(BP_block::min_op_ex_pos8_c);
}


BitArray str2bits(const string& s) {
	BitArray inp = BitArrayBuilder::create(s.length());
	for (size_t i = 0; i < s.length(); i++)
		if (s[i] == '(' || s[i] == '1') inp.setbit(i, true);
		else inp.setbit(i, false);
		return inp;
}

BitArray ui642bits(uint64_t v, int len) {
	assert(len > 0);
	BitArray inp = BitArrayBuilder::create(len);
	inp.setword(0, v);
	return inp;
}

TEST(BP_tree, find_pioneer1) {
	// ((()(()( )(()))(( (((()))) ()())()( ())))())
	// 10001000 00000111 00000000 00001001 00110001
	string bps = "((()(()()(()))(((((())))()())()(())))())";
	string exp = "1000100000000111000000000000100100110001";
	BitArray inp = str2bits(bps);

	BitArray out = find_pioneers(&inp, 8);
	for (int i = 0; i< bps.length(); ++i) {
		ASSERT_EQ((exp[i] == '1'), out[i]);
	}
}

TEST(BP_tree, test_find_pioneer2) {
	// ()(()())-(())()()
	string bps = "()(()())(())()()";
	string exp = "0000000000000000";
	BitArray inp = str2bits(bps);

	BitArray out = find_pioneers(&inp, 8);
	for (int i = 0; i< bps.length(); ++i) {
		ASSERT_EQ((exp[i] == '1'), out[i]);
	}
}

TEST(BP_tree, blk1) {
		{
			BitArray b = str2bits(")");
			BP_block block(b, 8);
			ASSERT_EQ(0, block.forward_scan(0, -1));
		}
	{
		BitArray b = str2bits("())");
		BP_block block(b, 8);
		ASSERT_EQ(2, block.forward_scan(0, -1));
	}
	{
		BitArray b = str2bits("))()()()))))))()");;
		BP_block block(b, 16);
		ASSERT_EQ(0, block.forward_scan(0, -1));
	}
}

TEST(BP_tree, blk2) {
	for (int i = 0; i < 65536; ++i) {
		BitArray b = ui642bits(i, 16);
		BP_block block(b, 16);
		if (block.forward_scan_slow(0, -1) != block.forward_scan(0, -1)) {
			ASSERT_EQ(block.forward_scan_slow(0, -1), block.forward_scan(0, -1));
		}
		if (block.forward_scan_slow(1, -1) != block.forward_scan(1, -1)) {
			ASSERT_EQ(block.forward_scan_slow(1, -1), block.forward_scan(1, -1));
		}
		ASSERT_EQ(block.backward_scan_slow(15, 1), block.backward_scan(15, 1));
		ASSERT_EQ(block.min_excess_pos(0, 7), block.min_excess_pos_slow(0, 7));
	}
}

TEST(BP_tree, blk3) {
	for (int i = 0; i < 200000; i++) {
		uint64_t v = rand64();
		BitArray b = ui642bits(v, 64);
		BP_block block(b, 64);
		int idx = rand() % 64;
		int bal = 1 + rand() % 16;
		ASSERT_EQ(block.forward_scan_slow(idx, -bal), block.forward_scan(idx, -bal));
		ASSERT_EQ(block.backward_scan_slow(idx, bal), block.backward_scan(idx, bal));

		int l = rand() % 31;
		int r = rand() % 32 + 32;
		ASSERT_EQ(block.min_excess_pos_slow(l, r), block.min_excess_pos(l, r));
	}
}

BitArray generate_BPS(size_t len) {
	assert(len % 2 == 0);
	BitArray out = BitArrayBuilder::create(len);
	size_t ex = 0, op = len/2, cl = len/2;

	for (size_t i = 0; i < len; ++i) {
		bool bit;
		if (ex == 0) bit = true;
		else {
			bit = !((rand32() % (op + cl)) < cl);
		}
		if (bit) {
			out.setbit(i, true);
			ex++;
			op--;
		} else {
			out.setbit(i, false);
			ex--;
			cl--;
		}
	}
	return out;
}

void test_near_bp(int blksize = 128) {
	BitArray b = generate_BPS(blksize);
	stack<int> pos;
	vector<int> match(blksize);
	vector<int> enclose(blksize);
	for (unsigned i = 0; i < b.length(); i++)
		if (b[i]) {
		if (!pos.empty()) enclose[i] = pos.top();
		else enclose[i] = -1;
		pos.push(i);
		} else {
		int t = pos.top();
		pos.pop();
		match[i] = t;
		match[t] = i;
		}
	BP_block block(b, blksize);
	for (int i = 0; i < b.length(); i++) {
		if (b[i]) {
			ASSERT_EQ(match[i], block.forward_scan(i, 0));
			if (enclose[i] >= 0) {
				ASSERT_EQ(enclose[i], block.backward_scan(i - 1, 1));
			} else {
				if (i > 0)
					ASSERT(BP_block::NOTFOUND == block.backward_scan(i - 1, 1));
			}
		} else {
			ASSERT_EQ(match[i], block.backward_scan(i, 0));
		}
	}
}

void test_bp(const BitArray& b, int blksize) {
	stack<int> pos;
	vector<int> match(b.length());
	vector<int> enclose(b.length());
	for (unsigned i = 0; i < b.length(); i++)
		if (b[i]) {
			if (!pos.empty()) enclose[i] = pos.top();
			else enclose[i] = -1;
			pos.push(i);
		} else {
			int t = pos.top();
			pos.pop();
			match[i] = t;
			match[t] = i;
			enclose[i] = t;
		}
	BP_aux bps;
	bps.build(b, blksize);
	for (int i = 0; i < b.length(); i++) {
		if (b[i]) {
			ASSERT_EQ(match[i], bps.find_close(i));
		} else {
			ASSERT_EQ(match[i], bps.find_open(i));
		}
		if (enclose[i] >= 0) {
			ASSERT_EQ(enclose[i], bps.enclose(i));
		} else {
			ASSERT(BP_block::NOTFOUND == bps.enclose(i));
		}
	}
}

void test_bp2(const BitArray& b, int blksize) {
	stack<int> pos;
	vector<int> match(b.length());
	vector<int> enclose(b.length());
	for (unsigned i = 0; i < b.length(); i++)
		if (b[i]) {
			if (!pos.empty()) enclose[i] = pos.top();
			else enclose[i] = -1;
			pos.push(i);
		} else {
			int t = pos.top();
			pos.pop();
			match[i] = t;
			match[t] = i;
			enclose[i] = t;
		}
	BP_aux bps;
	bps.build(b, blksize);
	OFileArchive2 fo;
	string fn = utils::tempfname();
	fo.open_write(fn);
	bps.save(fo);
	bps.clear();
	fo.close();
	IFileArchive2 fi;
	fi.open_read(fn);
	bps.load(fi);
	fi.close();

	for (int i = 0; i < b.length(); i++) {
		if (b[i]) {
			ASSERT_EQ(match[i], bps.find_close(i));
		} else {
			ASSERT_EQ(match[i], bps.find_open(i));
		}
		if (enclose[i] >= 0) {
			ASSERT_EQ(enclose[i], bps.enclose(i));
		} else {
			ASSERT(BP_block::NOTFOUND == bps.enclose(i));
		}
	}
	std::remove(fn.c_str());
}


TEST(BP_tree, bp1) {
	string bps = "((()(()()(()))(((((())))()())()(())))())";
	string exp = "1000100000000111000000000000100100110001";
	BitArray inp = str2bits(bps);
	test_bp(inp, 8);
}

void test_rr_enclose() {
	const int len = 64;
	const int blksize = 8;
	BitArray b = generate_BPS(len);
	BP_aux bps;
	bps.build(b, blksize);
	BP_block blkx(b, len);
	for (int i = 0; i < 20; i++) {
		int l = rand() % 32;
		int r = rand() % 32 + 32;
		uint64_t exp = blkx.min_excess_pos(l, r);
		if (exp != bps.min_excess_pos(l, r)) {
			uint64_t unexp = bps.min_excess_pos(l, r);
			ASSERT_EQ(exp, blkx.min_excess_pos_slow(l, r));
			cout << bps.to_str() << endl;
			ASSERT_EQ(exp, bps.min_excess_pos(l, r));
		}
	}
}

TEST(BP_tree, rr_enclose1) {
	//((()))((-()))((()-())((())-))((()((-))(())()-(())))((-)())()((-)()())()
	//      1     111     11     111   1   1            11     1  1       1
	//()(()())-(())()()
	BitArray b = str2bits("((()))((()))((()())((())))((()(())(())()(())))(()())()(()()())()");
	BP_aux bps;
	bps.build(b, 8);
	int l = 18;
	int r = 50;
	ASSERT_EQ(46, bps.min_excess_pos(l, r));
}

void testsize() {
	unsigned int len = 20000000;
	BitArray b = generate_BPS(len);
	BP_aux bps;
	bps.build(b, 256);
	OSizeEstArchive ar;
	bps.save(ar);
	cout.imbue(std::locale(cout.getloc(), new comma_numpunct()));
	cout << ar.opos()*8 << endl;
	cout << ((double)ar.opos()*8 - len)/len << endl;
	uint64_t rs = 1;
	clock_t st, ed;
	st = clock();
	for (size_t i = 0; i < bps.length(); ++i)
		rs |= bps.enclose(i);
	ed = clock();
	double time = ((double)(ed - st))/CLOCKS_PER_SEC;
	cout << "  qps = " << bps.length() / time << endl;
	if (rs) cout << ' ';
}


TEST(BP_tree, cart) {
	string bps = "(()(()(()))(()(()))())";
	uint64_t seq[11] = {0, 3, 2, 4, 2, 3, 1, 3, 1, 2, 0};
	vector<uint64_t> v(11);
	for (size_t i = 0; i < 11; i++)
		v[i] = seq[i];
	BitArray b = build_supercartisian_tree(true, v.begin(), v.end());
	ASSERT_EQ(bps.length(), b.length());
	for (size_t i = 0; i < bps.length(); ++i)
		ASSERT_EQ((bps[i] == '('), b[i]);
	RMQ_sct rmq;
	rmq.build(v);
	//cout << rmq.m_idx(3,6) << endl;
}

TEST(BP_tree, random_long) {
	for (int i = 0; i < 100; i++) {
		test_bp2(generate_BPS(1000), 16);
		if (i % 10 == 0) cout << '.';
	}

	for (int i = 0; i < 5000; i++) {
		if (i % 1000 == 0) cout << '.';
		test_rr_enclose();
	}


	test_bp(str2bits("()(()())(())()()"), 8);


	for (int i = 0; i < 20000; i++) {
		test_near_bp();
		if (i % 1000 == 0) cout << '.';
	}

	for (int i = 0; i < 10000; i++) {
		test_bp(generate_BPS(100), 16);
		if (i % 1000 == 0) cout << '.';
	}
	for (int i = 0; i < 5000; i++) {
		test_bp(generate_BPS(1000), 16);
		if (i % 1000 == 0) cout << '.';
	}
	cout << endl;
}

}//namespace

/*
int main() {
	testsize();
	test_all();
	//print_ex();
	return 0;
}*/
