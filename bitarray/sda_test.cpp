
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
	assert(SDArrayQuery::NOTFOUND == arr.find(psum));
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



void test_SDA_all() {
	test_SDA1();
	for (int i  = 0; i < 100; i++)
		test_SDA2();
}


int main() {
	test_SDA_all();
	return 0;
}