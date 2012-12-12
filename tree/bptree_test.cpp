#include "BP_bits.h"

#include <iomanip>
#include <stack>
#include <vector>
using namespace std;
using namespace mscds;

void print_minex() {
	cout << "{";
	for (int i = 0; i < 255; i++) {
		if (i > 0 && i % 16 == 0) cout << endl;
		cout << setw(2) << (int)BP_block::min_excess8_c(i) << ',';
	}
	cout << setw(2) << (int)BP_block::min_excess8_c(255) << '}' << endl;
}

void print_maxex() {
	cout << "{";
	for (int i = 0; i < 255; i++) {
		if (i > 0 && i % 16 == 0) cout << endl;
		cout << setw(2) << (int)BP_block::min_revex8_c(i) << ',';
	}
	cout << setw(2) << (int)BP_block::min_revex8_c(255) << '}' << endl;
}

void print_ex() {
	cout << "{";
	for (int i = 0; i < 255; i++) {
		if (i > 0 && i % 16 == 0) cout << endl;
		cout << setw(2) << (int)BP_block::excess8_c(i) << ',';
	}
	cout << setw(2) << (int)BP_block::excess8_c(255) << '}' << endl;
}


BitArray str2bits(const string& s) {
	BitArray inp = BitArray::create(s.length());
	for (size_t i = 0; i < s.length(); i++)
		if (s[i] == '(' || s[i] == '1') inp.setbit(i, true);
		else inp.setbit(i, false);
	return inp;
}

BitArray ui642bits(uint64_t v, int len) {
	assert(len > 0);
	BitArray inp = BitArray::create(len);
	inp.word(0) = v;
	return inp;
}

void test_find_pioneer1() {
	// ((()(()( )(()))(( (((()))) ()())()( ())))())
	// 10001000 00000111 00000000 00001001 00110001
	string bps = "((()(()()(()))(((((())))()())()(())))())";
	string exp = "1000100000000111000000000000100100110001";
	BitArray inp = str2bits(bps);

	BitArray out = find_pioneers(inp,8);
	for (int i = 0; i< 40; ++i) {
		assert((exp[i] == '1') == out[i]);
	}
}

uint64_t rand64() {
	return 
	  (((uint64_t) rand() <<  0) & 0x000000000000FFFFull) | 
	  (((uint64_t) rand() << 16) & 0x00000000FFFF0000ull) | 
	  (((uint64_t) rand() << 32) & 0x0000FFFF00000000ull) |
	  (((uint64_t) rand() << 48) & 0xFFFF000000000000ull);
}

void test_blk1() {
	{
		BitArray b = str2bits(")");
		BP_block block(b, 8);
		assert(0 == block.forward_scan(0,-1));
	}
	{
		BitArray b = str2bits("())");
		BP_block block(b, 8);
		assert(2 == block.forward_scan(0,-1));
	}
	{
		BitArray b = str2bits("))()()()))))))()");;
		BP_block block(b, 16);
		assert(0 == block.forward_scan(0,-1));
	}
}

void test_blk2() {
	for (int i = 0; i < 65536; ++i) {
		BitArray b = ui642bits(i, 16);
		BP_block block(b, 16);
		if (block.forward_scan_slow(0, -1) != block.forward_scan(0,-1)) {
			assert(block.forward_scan_slow(0, -1) ==  block.forward_scan(0,-1));
		}
		if (block.forward_scan_slow(1, -1) != block.forward_scan(1,-1)) {
			assert(block.forward_scan_slow(1, -1) ==  block.forward_scan(1,-1));
		}
		assert(block.backward_scan_slow(15, 1) == block.backward_scan(15, 1));
	}
}

void test_blk3() {
	for (int i = 0; i < 200000; i++) {
		uint64_t v = rand64();
		BitArray b = ui642bits(v, 64);
		BP_block block(b, 64);
		int idx = rand() % 64;
		int bal = 1 + rand() % 16;
		assert(block.forward_scan_slow(idx, -bal) == block.forward_scan(idx, -bal));
		assert(block.backward_scan_slow(idx, bal) == block.backward_scan(idx, bal));
	}
}

BitArray generate_BPS(size_t len) {
	assert(len % 2 == 0);
	BitArray out = BitArray::create(len);
	size_t ex = 0, op = len/2, cl = len/2;
	
	for (size_t i = 0; i < len; ++i) {
		bool bit;
		if (ex == 0) bit = true;
		else {
			bit = !((rand() % (op + cl)) < cl);
		}
		if (bit) {
			out.setbit(i, true);
			ex++;
			op--;
		}else {
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
			assert(match[i] == block.forward_scan(i, 0));
			if (enclose[i] >= 0) {
				assert(enclose[i] == block.backward_scan(i-1, 1));
			}else {
				if (i > 0) 
					assert(BP_block::NOTFOUND == block.backward_scan(i-1, 1));
			}
		}else {
			assert(match[i] == block.backward_scan(i, 0));
		}
	}
}

void test_bp(BitArray& b, int blksize) {
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
			assert(match[i] == bps.find_close(i));
		}else {
			assert(match[i] == bps.find_open(i));
		}
		if (enclose[i] >= 0) {
			assert(enclose[i] == bps.enclose(i));
		}else {
			assert(BP_block::NOTFOUND == bps.enclose(i));
		}
	}
}

void test_bp1() {
	string bps = "((()(()()(()))(((((())))()())()(())))())";
	string exp = "1000100000000111000000000000100100110001";
	BitArray inp = str2bits(bps);
	test_bp(inp, 8);
}

void test_all() {
	test_find_pioneer1();
	test_blk1();
	test_blk2();
	test_blk3();
	for (int i = 0; i < 50000; i++)
		test_near_bp();
	test_bp1();
	for (int i = 0; i < 50000; i++)
		test_bp(generate_BPS(100), 16);
}

int main() {
	//test1();
	//print_maxex();
	//test2();
	//test1a();
	//test3();
	test_all();
	return 0;
}