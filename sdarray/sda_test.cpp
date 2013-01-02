
#include "sdarray.h"
#include "mem/filearchive.h"
#include "utils/utest.h"
#include "utils/file_utils.h"
#include "utils/utest.h"
#include "sdarray_sml.h"

#include <vector>
#include <cassert>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>
#include <stdint.h>

using namespace std;
using namespace mscds;

string get_tempdir() {
	return utils::get_temp_path();
};

void test_sdarray_zeros(){
	SDArrayBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i){
		bd.add(0);
	}
	SDArrayQuery sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i){
		ASSERT_EQ(0, sda.prefixsum(i));
		//uint64_t val = 0;
		ASSERT_EQ(0, sda.lookup(i));
		//ASSERT_EQ(0, val);
	}

	//ASSERT_EQ(SDArrayQuery::NOTFOUND, sda.find(0));
}

void test_sdarray_ones(){
	SDArrayBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i){
		bd.add(1);
	}
	SDArrayQuery sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i){
		ASSERT_EQ(i, sda.prefixsum(i));
		//uint64_t val = 0;    
		ASSERT_EQ(1, sda.lookup(i));
		//ASSERT_EQ(i, );
	}

	for (int i = 0; i < N; ++i){
		//ASSERT_EQ(i, sda.find(i));
	}
}

void test_sdarray_increasing(){
	SDArrayBuilder bd;
	SDArrayQuery sda;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		cums[i] = sum;
		bd.add(i);
		sum += i;
	}
	cums[N] = sum;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (uint64_t i = 0; i < N; ++i){
		ASSERT_EQ(cums[i], sda.prefixsum(i));
		//uint64_t val = 0;
		//ASSERT_EQ(cums[i], sda.prefixsumLookup(i, val));
		//ASSERT_EQ(i, val);
		ASSERT_EQ(i, sda.lookup(i));
	}

	for (uint64_t i = 0 ;i < sum; i += 17){
		vector<uint64_t>::iterator it = upper_bound(cums.begin(), cums.end(), i);
		size_t ind = it - cums.begin() - 1;

		ASSERT(i >= cums[ind]);
		if (ind+1 < cums.size()){
			ASSERT(i <= cums[ind+1]);
		}

		//ASSERT_EQ(ind, sda.find(i));
	}
}



void test_sdarray_random(){
	SDArrayBuilder bd;
	SDArrayQuery sda;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		cums[i] = sum;
		vals[i] = rand();
		bd.add(vals[i]);
		sum += vals[i];
	}
	cums[N] = sum;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		ASSERT_EQ(sum, sda.prefixsum(i));
		ASSERT_EQ(vals[i], sda.lookup(i));
		sum += vals[i];
	}

	uint64_t M = 100;
	for (uint64_t i = 0; i < M; ++i){
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = upper_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin() - 1;

		ASSERT(val >= cums[ind]);
		if (ind+1 < cums.size()){
			ASSERT(val <= cums[ind+1]);
		}

		//ASSERT_EQ(ind, sda.find(val));
	}
}



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
		ASSERT(A[i-1] == val);
	}
	int psum = 0;
	for (int i = 0; i < len-1; i++) {
		psum += A[i];
		//ASSERT(i+1 == arr.find(psum));
		//ASSERT(i == arr.find(psum-1));
	}
	psum += A[len-1];
	//ASSERT(1000 == arr.find(psum));
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
		ASSERT(A[i-1] == val);
	}
	int psum = 0;
	for (int i = 0; i < len - 1; i++) {
		psum += A[i];
		//ASSERT(i+1 == d2.find(psum));
		//ASSERT(i == d2.find(psum-1));
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
		ASSERT(vec[i] == v.bit(i));

	SDRankSelect r;
	r.build(v);

	for (int i = 0; i <= vec.size(); ++i)
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT(ranks[i] == r.rank(i));
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
			ASSERT(vec[pos] == true);
		}
		ASSERT(pos > last);
		last = pos;
	}
	cout << ".";
}

void test_SDA_all() {
	test_sdarray_zeros();
	test_sdarray_ones();
	test_sdarray_increasing();
	test_sdarray_random();
	for (int i = 0; i < 200; i++)
		test_rank(1000);
	test_SDA1();
	for (int i  = 0; i < 100; i++)
		test_SDA2();
	cout << endl;
}
BitArray randbit(unsigned int len, unsigned int & cnt) {
	BitArray b = BitArray::create(len);
	cnt = 0;
	for (size_t i = 0; i < len; ++i)
		if (rand() % 100 < 10) { b.setbit(i, true); cnt++;}
		else b.setbit(i, false);
	return b;
}

template<typename T>
size_t get_bit_size(const T& t) {
	OSizeEstArchive ar;
	t.save(ar);
	return ar.opos() * 8;
}

void testspeed() {
	srand(0);
	unsigned int len = 10000000, oc;
	BitArray b = randbit(len, oc);
	SDRankSelectSml rs;
	rs.build(b);
	cout << "ones = " << oc << endl;
	cout << get_bit_size(rs) << endl;
	clock_t st = std::clock();
	uint64_t val = 3;
	unsigned int nqueries = 1000000;
	for (size_t i = 0; i < nqueries; i++) {
		uint64_t r = rs.rank(rand() % (len));
		val ^= rs.select(r);
	}
	clock_t ed = std::clock();
	if (val) cout << ' ';
	double t = ((double)(ed - st) / CLOCKS_PER_SEC);
	cout << nqueries / t << endl;
}

//----------------------------------------------------------------------

void test_sda2_rand() {
	SDArraySmlBuilder bd;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		cums[i] = sum;
		vals[i] = rand();
		bd.add(vals[i]);
		sum += vals[i];
	}
	cums[N] = sum;
	SDArraySml sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		uint64_t v = sda.prefixsum(i);
		ASSERT_EQ(cums[i], v);
		ASSERT_EQ(vals[i], sda.lookup(i));
	}

	uint64_t M = 100;
	for (uint64_t i = 0; i < M; ++i){
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin();

		if (ind < cums.size())
			ASSERT(val <= cums[ind]);
		ASSERT_EQ(ind, sda.rank(val));
	}
}

void test_sda2_rand2() {
	SDArraySmlBuilder bd;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		cums[i] = sum;
		vals[i] = rand() % 3;
		bd.add(vals[i]);
		sum += vals[i];
	}
	cums[N] = sum;
	SDArraySml sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		uint64_t v = sda.prefixsum(i);
		ASSERT_EQ(cums[i], v);
		ASSERT_EQ(vals[i], sda.lookup(i));
	}

	uint64_t M = 100;
	for (uint64_t i = 0; i < M; ++i){
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin();

		if (ind < cums.size())
			ASSERT(val <= cums[ind]);
		if (ind != sda.rank(val)) {
			auto v = sda.rank(val);
			ASSERT_EQ(ind, sda.rank(val));
		}
	}
}


void test_sda2_inc() {
	SDArraySmlBuilder bd;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i){
		cums[i] = sum;
		vals[i] = i;
		bd.add(vals[i]);
		sum += vals[i];
	}
	cums[N] = sum;
	SDArraySml sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		uint64_t v = sda.prefixsum(i);
		ASSERT_EQ(cums[i], v);
		ASSERT_EQ(vals[i], sda.lookup(i));
	}

	uint64_t M = 100;
	for (uint64_t i = 0; i < M; ++i){
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin();

		if (ind < cums.size())
			ASSERT(val <= cums[ind]);
		ASSERT_EQ(ind, sda.rank(val));
	}
}


void test_sda2_ones() {
	SDArraySmlBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i)
		bd.add(1);
	SDArraySml sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		uint64_t v = sda.prefixsum(i);
		ASSERT_EQ(i, v);
		if (1 != sda.lookup(i)) {
			ASSERT_EQ(1, sda.lookup(i));
		}
	}
	for (int i = 0; i < N; ++i){
		if (i != sda.rank(i)) {
			auto x = sda.rank(i);
			ASSERT_EQ(i, sda.rank(i));
		}
	}

}

void test_sda2_zeros() {
	SDArraySmlBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i)
		bd.add(0);
	SDArraySml sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		uint64_t v = sda.prefixsum(i);
		ASSERT_EQ(0, v);
		ASSERT_EQ(0, sda.lookup(i));
	}

}


void test_rank2(int len) {
	std::vector<bool> vec;
	for (int i = 0; i < len; ++i) {
		if (rand() % 20 == 1)
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
		ASSERT(vec[i] == v.bit(i));

	SDRankSelectSml r;
	r.build(v);

	for (int i = 0; i <= vec.size(); ++i)
		if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT(ranks[i] == r.rank(i));
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
			ASSERT(vec[pos] == true);
		}
		ASSERT(pos > last);
		last = pos;
	}
	cout << ".";
}

void test_rank3(int len) {
	std::vector<bool> vec;
	for (int i = 0; i < len; ++i) {
		if (rand() % 20 == 1)
			vec.push_back(true);
		else vec.push_back(false);
	}

	BitArray v;
	v = BitArray::create(vec.size());
	v.fillzero();
	for (unsigned int i = 0; i < vec.size(); i++)
		v.setbit(i, vec[i]);

	for (unsigned int i = 0; i < vec.size(); i++)
		ASSERT(vec[i] == v.bit(i));

	SDRankSelectSml r;
	r.build(v);


	for (unsigned int i = 0; i < vec.size(); ++i)
		if (vec[i])
			ASSERT_EQ(i, r.select(r.rank(i)));
	cout << ".";
}

int main() {
	//testspeed();
	//return 0;
	for (int i = 0; i < 200; i++) 
		test_rank3(1020 + rand() % 8);
	test_sda2_ones();
	test_sda2_zeros();
	test_sda2_inc();
	for (int i = 0; i < 200; i++) 
		test_rank2(1020 + rand() % 8);
	for (int i = 0; i < 100; i++) {
		test_sda2_rand();
		test_sda2_rand2();
	}
	//return 0;
	test_SDA_all();
	return 0;
}