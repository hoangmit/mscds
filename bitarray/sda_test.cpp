
#include "sdarray.h"
#include "mem/filearchive.h"
#include <vector>
#include <cassert>
#include <fstream>
#include <string>
#include <stdint.h>

using namespace std;
using namespace mscds;

string get_tempdir() {
	return "../tmp/";
}; // "/tmp/

void test_SDA1() {
	const int len = 1000;
	const int range = 100;
	vector<unsigned int> A(len), S(len);
	int sum = 0;
	for (int i = 0; i < len; ++i) {
		A[i] = (rand() % range) + 1; // number must > 0
		sum += A[i];
	}
	SDArrayQuery arr;
	SDArrayBuilder bd;
	
	for (int i = 0; i < len; ++i)
		bd.add(A[i]);
	bd.build(&arr);

	for (int i = 1; i <= len; i++) {
		int val = arr.prefixsum(i) - arr.prefixsum(i-1);
		assert(A[i-1] == val);
	}
	int psum = 0;
	for (int i = 0; i < len-1; i++) {
		psum += A[i];
		if (i+1 != arr.find(psum)) {
			cout << i+1 << " " << arr.find(psum) << endl;
			assert(i+1 == arr.find(psum));
		}
		if (i != arr.find(psum-1)) {
			assert(i == arr.find(psum-1));
		}
	}
	psum += A[len-1];
	assert(1000 == arr.find(psum));
	cout << ".";
}


void test_SDA2() {
	const int len = 10000;
	const int range = 100;
	vector<unsigned int> A(len), S(len);
	int sum = 0;
	for (int i = 0; i < len; ++i) {
		A[i] = (rand() % range) + 1; // number must > 0
		sum += A[i];
	}
	SDArrayBuilder bd;

	for (int i = 0; i < len; ++i)
		bd.add(A[i]);
	OFileArchive ofa;
	ofa.open_write(get_tempdir() + string("tmp_saveload"));
	bd.build(ofa);
	ofa.close();

	IFileArchive ifa;
	SDArrayQuery d2;
	ifa.open_read(get_tempdir() + "tmp_saveload");
	d2.load(ifa);
	ifa.close();

	for (int i = 1; i <= len; i++) {
		int val = d2.prefixsum(i) - d2.prefixsum(i-1);
		assert(A[i-1] == val);
	}
	int psum = 0;
	for (int i = 0; i < len - 1; i++) {
		psum += A[i];
		if (i+1 != d2.find(psum)) {
			cout << i+1 << " " << d2.find(psum) << endl;
			assert(i+1 == d2.find(psum));
		}
		if (i != d2.find(psum-1)) {
			assert(i == d2.find(psum-1));
		}
	}
	cout << ".";
}


void test_rank(int len) {
	std::vector<bool> vec;
	for (int i = 0; i < len; ++i) {
		if (rand() % 50 == 1)
			vec.push_back(true);
		else vec.push_back(false);
	}

	vector<int> ranks(vec.size() + 1);
	ranks[0] = 0;
	for (unsigned int i = 1; i <= vec.size(); i++)
		if (vec[i-1]) ranks[i] = ranks[i-1] + 1;
		else ranks[i] = ranks[i-1];
	BitArray v;
	v = BitArray::create(vec.size());
	v.fillzero();
	for (unsigned int i = 0; i < vec.size(); i++)
		v.setbit(i, vec[i]);

	for (unsigned int i = 0; i < vec.size(); i++)
		assert(vec[i] == v.bit(i));

	SDRankSelect r;
	r.build(v);

	for (int i = 0; i <= vec.size(); ++i)
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			assert(ranks[i] == r.rank(i));
		}
	unsigned int onecnt = 0;
	for (unsigned int i = 0; i < vec.size(); ++i)
		if (vec[i]) onecnt++;
	int last = -1;
	for (unsigned int i = 0; i < onecnt; ++i) {
		int pos = r.select(i);
		if (pos >= vec.size() || !vec[pos] || pos <= last) {
			cout << "select " << i << "  " << r.select(i) << endl;
			if (i > 0) r.select(i-1);
			assert(vec[pos] == true);
		}
		assert(pos > last);
		last = pos;
	}
	cout << ".";
}



void test_SDA_all() {
	test_rank(1000);
	test_SDA1();
	for (int i  = 0; i < 100; i++)
		test_SDA2();
	cout << endl;
}


int main() {
	test_SDA_all();
	return 0;
}