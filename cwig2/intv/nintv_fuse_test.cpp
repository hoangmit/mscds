
#include "utils/utest.h"
#include "nintv_fuse.h"
#include "cwig/intv/intv_rand_gen.hpp"

#include "utils/benchmark.h"

#include <iostream>
#include <fstream>

using namespace mscds;
using namespace std;
using namespace app_ds;

template<typename T>
void test_intervals_easy(const std::vector<std::pair<unsigned int, unsigned int> >& rng, const T& r, int testid = 0) {
	ASSERT_EQ(rng.size(), r.length());
	size_t pslen = 0;
	for (size_t i = 0; i < rng.size(); ++i) {
		ASSERT_EQ(rng[i].first, r.int_start(i)) << "i == " << i;
		auto len = rng[i].second - rng[i].first;
		ASSERT_EQ(len, r.int_len(i)) << "i = " << i << std::endl;
		ASSERT_EQ(rng[i].second, r.int_end(i));
		ASSERT_EQ(pslen, r.int_psrlen(i)) << "i = " << i;
		pslen += len;
		auto px = r.int_startend(i);
		ASSERT(rng[i].first == px.first && rng[i].second == px.second);
	}
}

template<typename T>
void rank_cov(const std::vector<std::pair<unsigned int, unsigned int> >& rng, const T& r, int testid = 0) {
	size_t mlen = rng.back().second;
	size_t j = 0, jp = r.npos();
	size_t cnt = 0;
	for (size_t i = 0; i < mlen; ++i) {
		if (rng[j].second <= i) ++j;
		if (rng[j].first <= i) jp = j;
		auto v = r.rank_interval(i);
		ASSERT_EQ(jp, v);
		ASSERT_EQ(cnt, r.coverage(i)) << "test id = " << testid << "   i = " << i << endl;
		auto p = r.find_cover(i);
		if (rng[j].first <= i) {
			++cnt;
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(i - rng[j].first + 1, p.second);
		} else {
			ASSERT_EQ(j, p.first);
			ASSERT_EQ(0, p.second);
		}
	}
	ASSERT_EQ(rng.size() - 1, j);
	auto v = r.rank_interval(mlen);
	ASSERT_EQ(rng.size() - 1, v);
}

void test1() {
	vector<int> Av = {1, 1, 1, 0, 0, 9, 9, 2, 2, 2, 3};
	auto rng = convert2pair(genInp(Av));
	
	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test2() {
	vector<int> Av = {1, 1, 1, 0, 9, 9, 0, 2, 2, 2, 3};
	auto rng = convert2pair(genInp(Av));

	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test3(unsigned int len = 1024) {
	auto rng = gen_intv2(1024, 50, 50);
	NIntvFuseBuilder bd;
	bd.init();
	for (auto& p: rng) {
		bd.add(p.first, p.second);
	}

	NIntvFuseQuery qs;
	bd.build(&qs);
	test_intervals_easy(rng, qs.b);
	rank_cov(rng, qs.b);
}

void test4() {
	for (unsigned i = 0; i < 50; ++i) 
		test3(1024);
	for (unsigned i = 0; i < 30; ++i)
		test3(1025);
	for (unsigned i = 0; i < 20; ++i)
		test3(1023);
}

void test_all() {
	test1();
	test2();
	test3();
	test4();
}

#include "cwig/intv/nintv.h"

#include "mem/file_archive2.h"
#include "mem/fmap_archive2.h"
#include "remote_file/remote_archive2.h"
#include <ctime>

std::vector<std::pair<unsigned, unsigned> > read_file(const std::string& file) {
	std::vector<std::pair<unsigned, unsigned> > out;
	std::ifstream fi(file);
	while (fi) {
		std::string chr;
		unsigned int st, ed;
		fi >> chr >> st >> ed;
		if (!fi || chr != "chr1") break;
		out.emplace_back(st, ed);
		fi.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	fi.close();
	std::cout << "Read " << out.size() << " intervals" << std::endl;
	return out;
}

template<typename T>
void load_data(T& d, const std::string& input) {
	if (input.length() >= 8 && (input.substr(0, 7) == "http://" || input.substr(0, 8) == "https://")) {
		mscds::RemoteArchive2 rar;
		rar.open_url(input);
		d.load(rar);
	} else {
		mscds::IFileMapArchive2 fi;
		fi.open_read(input);
		d.load(fi);
		fi.close();
	}
	std::cout << "Loaded " << input << std::endl;
}

void build_normal(const std::string& inp, const std::string& ds) {
	auto pl = read_file(inp);
	PNIntvBuilder bd;
	for (const auto& p : pl) {
		bd.add(p.first, p.second);
	}
	PNIntv qs;
	bd.build(&qs);
	save_to_file(qs, ds);
}

void build_fusion(const std::string& inp, const std::string& ds) {
	auto pl = read_file(inp);
	NIntvFuseBuilder bd;
	bd.init();
	for (const auto& p : pl) {
		bd.add(p.first, p.second);
	}
	NIntvFuseQuery qs;
	bd.build(&qs);
	save_to_file(qs, ds);
}

void load_normal(const std::string& ds_file, const std::string& qs_file) {
	PNIntv qs;
	load_data(qs, ds_file);
	auto pl = read_file(qs_file);

	unsigned int x = 1;
	clock_t st_tm = std::clock();
	for (const auto& p : pl) {
		auto t1 = qs.rank_interval(p.first);
		auto t2 = qs.rank_interval(p.second);
		x = x ^ t1 ^ t2;
	}
	clock_t ed_tm = std::clock();
	if (x) std::cout << endl;
	std::cout << endl << (ed_tm - st_tm)*1000.0 / CLOCKS_PER_SEC << endl;
}

void load_fusion(const std::string& ds_file, const std::string& qs_file) {
	NIntvFuseQuery qs;
	load_data(qs, ds_file);
	auto pl = read_file(qs_file);

	unsigned int x = 1;
	clock_t st_tm = std::clock();
	for (const auto& p : pl) {
		auto t1 = qs.b.rank_interval(p.first);
		auto t2 = qs.b.rank_interval(p.second);
		x = x ^ t1 ^ t2;
	}
	clock_t ed_tm = std::clock();
	if (x) std::cout << endl;
	std::cout << endl << (ed_tm - st_tm)*100.0 / CLOCKS_PER_SEC << endl;
}

int run_exp(int argc, char* argv[]) {
	if (argc != 5) {
		std::cout << std::endl;
		std::cout << "program {B|R} {N|F} input_file1 input_file2" << std::endl;
		std::cout << "   B: build,  R: run,  N: normal,  F: fusion" << std::endl;
		std::cout << std::endl;
		return 1;
	}
	if (argv[1] == string("B")) {
		if (argv[2] == string("N")) {
			build_normal(argv[3], argv[4]);
		} else
		if (argv[2] == string("F")) {
			build_fusion(argv[3], argv[4]);
		} else return 1;
	} else
	if (argv[1] == string("R")) {
		if (argv[2] == string("N")) {
			load_normal(argv[3], argv[4]);
		} else
		if (argv[2] == string("F")) {
			load_fusion(argv[3], argv[4]);
		} else return 1;
	} else return 1;
	return 0;
}

//--------------------------------------------------------------------

#include "cwig/valrange.h"

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


#include "../fused_intval2.h"


void check(std::deque<ValRange>& all, IntValQuery2& qs, unsigned int testid = 0) {
	double ps = 0;
	for (size_t i = 0; i < all.size(); ++i) {
		double val = qs.range_value(i);
		assert(all[i].val == val);
		double sm = qs.range_psum(i);
		if (ps != sm) {
			std::cout << "wrong at i = " << i << std::endl;
			std::cout << "exp = " << ps << "   val = " << sm << std::endl;
		}
		assert(ps == sm);
		ps += (all[i].ed - all[i].st) * all[i].val;
	}
}

void test5(unsigned int len = 20, unsigned int testid = 0) {
	IntValBuilder2 bd;
	std::deque<ValRange> all;
	IntValQuery2 qs;
	auto vp = gen_intv2(len);
	all.resize(vp.size());
	for (unsigned int i = 0; i < vp.size(); ++i) {
		all[i].st = vp[i].first;
		all[i].ed = vp[i].second;
		all[i].val = rand() % 1000 + 1;
		bd.add(all[i].st, all[i].ed, all[i].val);
	}

	bd.build(&qs);

	check(all, qs, testid);
}

void test6() {
	test5();
	test5(70);
	for (unsigned int i = 0; i < 100; i++) {
		test5(1000, i);
		test5(1024, i);
	}
}
#include "mem/save_load_test.h"
void test_saveload() {
	IntValBuilder2 bd;
	std::deque<ValRange> all;
	IntValQuery2 qs, qs2;
	auto vp = gen_intv2(1000);
	all.resize(vp.size());
	for (unsigned int i = 0; i < vp.size(); ++i) {
		all[i].st = vp[i].first;
		all[i].ed = vp[i].second;
		all[i].val = rand() % 100 + 1;
		bd.add(all[i].st, all[i].ed, all[i].val);
	}

	std::string filename = utils::tempfname();
	OFileArchive2 fo;
	fo.open_write(filename);
	bd.build(&qs);
	qs.save(fo);
	fo.close();
	IFileMapArchive2 fi;
	fi.open_read(filename);
	qs2.load(fi);
	fi.close();
	check(all, qs2);
	std::remove(filename.c_str());
}

int main(int argc, char* argv[]) {
	//test_all();
	//load_fusion("C:/temp/bf.dat", "C:/temp/wgEn.broadPeak");
	//return run_exp(argc, argv);
	//rand();
	test6();
	test_saveload();
	
	return 0;
}

