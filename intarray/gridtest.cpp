
#include "wat_array.h"
#include <vector>
using namespace std;
using namespace mscds;

void test_watarr() {
	vector<uint64_t> inp;
	inp.push_back(1);
	inp.push_back(2);
	inp.push_back(1);
	inp.push_back(1);
	inp.push_back(2);
	inp.push_back(1);
	inp.push_back(1);
	inp.push_back(2);
	WatQuery arr;
	WatBuilder bd;
	bd.build(inp, &arr);
	assert(5 == arr.rank(1, 8));
}

void test_wat_1() {
	unsigned int arr[8] = {2, 7, 1, 7, 3, 0, 4, 4};
	vector<uint64_t> v(arr, arr+8);
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);
	assert("01010011" == wq.bit_layer(0).to_str());
	assert("10101100" == wq.bit_layer(1).to_str());
	assert("10010011" == wq.bit_layer(2).to_str());
	assert(2 == wq.access(0));
	assert(6 == wq.select(4,0));
}

void test_wat_access() {
	vector<uint64_t> v;
	int len = 1000;
	for (int i = 0; i < len; ++i)
		v.push_back(rand() % (len / 10));
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < len; ++i) {
		unsigned int testv = rand() % (len / 10);
		int count = 0;
		for (int j = 0; j < i; ++j)
			if (v[j] < testv) count++;
		unsigned int rv = wq.rankLessThan(testv, i);
		assert(count == rv);
		assert(v[i] == wq.access(i));
	}
}

void gridquerytest_x() {
	vector<uint64_t> inp;
	for (int i = 0; i < 1000; i++)
		inp.push_back(i);
	WatQuery arr;
	WatBuilder bd;
	bd.build(inp, &arr);

	GridQuery gq;
	vector<unsigned int> X, Y;
	for (int i = 0; i < 9; i++) {
		X.push_back((i+1)*100);
		Y.push_back((i+1)*100);
	}
	gq.process(&arr, X, Y);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			cout << gq.results[i][j] << "  ";
		}
		cout << endl;
	}
}

void gridquerytest1() {
	vector<uint64_t> inp;
	inp.push_back(0);
	inp.push_back(1);
	WatQuery arr;
	WatBuilder bd;
	bd.build(inp, &arr);

	GridQuery gq;
	vector<unsigned int> X, Y;
	X.push_back(1);
	X.push_back(2);
	Y.push_back(1);
	Y.push_back(2);
	gq.process(&arr, X, Y);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			cout << gq.results[i][j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
}


void gridquerytest2() {
	vector<uint64_t> inp;
	inp.push_back(1);
	inp.push_back(2);
	WatQuery arr;
	WatBuilder bd;
	bd.build(inp, &arr);

	uint64_t llt, lmt, r;
	arr.rankAll(2, 1, r, llt, lmt);
	assert(1 == llt);
	arr.rankAll(2, 2, r, llt, lmt);
	assert(1 == llt);
	arr.rankAll(2, 1, r, llt, lmt);
	assert(1 == llt+r);
	arr.rankAll(2, 2, r, llt, lmt);
	assert(2 == llt+r);

	GridQuery gq;
	vector<unsigned int> X, Y;
	X.push_back(1);
	X.push_back(2);
	Y.push_back(2);
	Y.push_back(3);
	gq.process(&arr, X, Y);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			cout << gq.results[i][j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
}

void wat_test_all() {
	test_wat_1();
	test_watarr();
	test_wat_access();
	gridquerytest_x();
}

int main() {
	wat_test_all();
	return 0;
}
