
#include "sdarray.h"
#include "sdarray_sml.h"
#include "sdarray_c.h"
#include "sdarray_zero.h"
#include "sdarray_rl.h"

#include "mem/file_archive2.h"
#include "mem/info_archive.h"

#include "utils/file_utils.h"
#include "utils/utest.h"


#include <vector>
#include <cassert>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>
#include <stdint.h>

namespace tests {

using namespace std;
using namespace mscds;

string get_tempdir() {
	return utils::get_temp_path();
};


template<typename SDArray>
static void check_all(const SDArray& sda, const SDArrayZero& zero) {
	ASSERT_EQ(zero.length(), sda.length());
	unsigned N = zero.length();
	// prefixsum
	unsigned exp, val;
	for (int i = 0; i < N; ++i) {
		exp = zero.prefixsum(i);
		val = sda.prefixsum(i);
		if (exp != val) {
			sda.prefixsum(i);
		}
		ASSERT_EQ(exp, val);
	}
	// lookup
	for (int i = 0; i < N; ++i) {
		exp = zero.lookup(i);
		val = sda.lookup(i);
		if (exp != val) {
			sda.lookup(i);
		}
		ASSERT_EQ(exp, val);
		uint64_t ps0, ps1;
		ASSERT_EQ(exp, sda.lookup(i, ps0));
		zero.lookup(i, ps1);
		ASSERT_EQ(ps0, ps1);
	}

	// lookup different order
	for (int i = 0; i < N; ++i) {
		unsigned j;
		if (i % 2 == 0) j = i; 
		else j = N - 1 - i;
		exp = zero.lookup(j);
		val = sda.lookup(j);
		ASSERT_EQ(exp, val);
	}

	//rank
	auto last = zero.prefixsum(N);
	if (last <= N*10) {
		for (unsigned i = 0; i <= last; ++i) {
			exp = zero.rank(i);
			val = sda.rank(i);
			ASSERT_EQ(exp, val);
		}
	} else {
		for (unsigned i = 0; i <= last; i += 19) {
			exp = zero.rank(i);
			val = sda.rank(i);
			if (exp != val) {
				sda.rank(i);
			}
			ASSERT_EQ(exp, val);
		}
	}
}
template<typename SDArray>
static void check_prefixsum_lookup(const std::vector<unsigned int>& vals) {
	typedef typename SDArray::BuilderTp BuilderTp;
	BuilderTp bd;
	unsigned int N = vals.size();
	SDArrayZero zero;
	for (unsigned int v : vals) {
		bd.add(v);
		zero.add(v);
	}
	SDArray sda;
	bd.build(&sda);
	ASSERT_EQ(N, zero.length());
	check_all<SDArray>(sda, zero);
}


static std::vector<unsigned int> gen_zeros(unsigned int N = 10000) {
	std::vector<unsigned int> ret(N);
	for (unsigned i = 0; i < N; ++i) ret[i] = 0;
	return ret;
}

static std::vector<unsigned int> gen_ones(unsigned int N = 10000) {
	std::vector<unsigned int> ret(N);
	for (unsigned i = 0; i < N; ++i) ret[i] = 1;
	return ret;
}

static std::vector<unsigned int> gen_same(unsigned val, unsigned int N = 10000) {
	std::vector<unsigned int> ret(N);
	for (unsigned i = 0; i < N; ++i) ret[i] = val;
	return ret;
}



static std::vector<unsigned int> gen_increasing(unsigned int N = 10000) {
	std::vector<unsigned int> ret(N);
	for (unsigned i = 0; i < N; ++i) ret[i] = i;
	return ret;
}

static std::vector<unsigned int> gen_rand(unsigned int N = 10000, unsigned range = 1000) {
	std::vector<unsigned int> ret(N);
	for (unsigned i = 0; i < N; ++i) ret[i] = rand() % range;
	return ret;
}

static std::vector<unsigned int> gen_rand2(unsigned int N = 3000, unsigned range = 1000, unsigned zero_len=4) {
	std::vector<unsigned int> ret;
	unsigned p = 0;
	for (unsigned i = 0; i < N; ++i) {
		ret.push_back(rand() % range);
		unsigned cc = rand() % zero_len;
		for (unsigned j = 0; j < cc; ++j)
			ret.push_back(0);
	}
	return ret;
}

TEST(sdatest, sdarray_zeros) {
	SDArrayBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i) {
		bd.add(0);
	}
	SDArrayQuery sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		ASSERT_EQ(0, sda.prefixsum(i));
		//uint64_t val = 0;
		ASSERT_EQ(0, sda.lookup(i));
		//ASSERT_EQ(0, val);
	}

	//ASSERT_EQ(SDArrayQuery::NOTFOUND, sda.find(0));
}

TEST(sdatest, sdarray_ones) {
	SDArrayBuilder bd;
	int N = 10000;
	for (int i = 0; i < N; ++i) {
		bd.add(1);
	}
	SDArrayQuery sda;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (int i = 0; i < N; ++i) {
		ASSERT_EQ(i, sda.prefixsum(i));
		//uint64_t val = 0;    
		ASSERT_EQ(1, sda.lookup(i));
		//ASSERT_EQ(i, );
	}

	for (int i = 0; i < N; ++i) {
		//ASSERT_EQ(i, sda.find(i));
	}
}

TEST(sdatest, sdarray_increasing) {
	SDArrayBuilder bd;
	SDArrayQuery sda;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		cums[i] = sum;
		bd.add(i);
		sum += i;
	}
	cums[N] = sum;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	for (uint64_t i = 0; i < N; ++i) {
		ASSERT_EQ(cums[i], sda.prefixsum(i));
		//uint64_t val = 0;
		//ASSERT_EQ(cums[i], sda.prefixsumLookup(i, val));
		//ASSERT_EQ(i, val);
		ASSERT_EQ(i, sda.lookup(i));
	}

	/*for (uint64_t i = 0; i < sum; i += 17) {
		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), i);
		size_t ind = it - cums.begin();

		ASSERT(i <= cums[ind]);
		//if (ind > 0) {
		//	ASSERT(i > cums[ind-1]);
		//}

		ASSERT_EQ(ind, sda.rank(i));
	}*/
}



TEST(sdatest, sdarray_random) {
	SDArrayBuilder bd;
	SDArrayQuery sda;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		cums[i] = sum;
		vals[i] = rand();
		bd.add(vals[i]);
		sum += vals[i];
	}
	cums[N] = sum;
	bd.build(&sda);

	ASSERT_EQ(N, sda.length());
	sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
		ASSERT_EQ(sum, sda.prefixsum(i));
		ASSERT_EQ(vals[i], sda.lookup(i));
		sum += vals[i];
	}

	/*uint64_t M = 100;
	for (uint64_t i = 0; i < M; ++i) {
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin() - 1;

		ASSERT(val >= cums[ind]);
		if (ind+1 < cums.size()) {
			ASSERT(val <= cums[ind+1]);
		}

		ASSERT_EQ(ind, sda.rank(val));
	}*/
}



TEST(sdatest, SDA1) {
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
}


void test_SDA2_rnd() {
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
	OFileArchive2 ofa;
	string fname = utils::tempfname();
	ofa.open_write(fname);
	bd.build(ofa);
	ofa.close();

	IFileArchive2 ifa;
	SDArrayQuery d2;
	ifa.open_read(fname);
	d2.load(ifa);
	ifa.close();

	for (int i = 1; i <= len; i++) {
		int val = d2.prefixsum(i) - d2.prefixsum(i-1);
		ASSERT(A[i-1] == val);
	}
	/*
	int psum = 0;
	for (int i = 0; i < len - 1; i++) {
	psum += A[i];
	ASSERT(i+1 == d2.find(psum));
	ASSERT(i == d2.find(psum-1));
	}*/
	d2.clear();
	std::remove(fname.c_str());
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
		v = BitArrayBuilder::create(vec.size());
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
}

TEST(sdatest, SDA_rnd_all) {
	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("test_rank");
		test_rank(1000);
		if (i % 40 == 0) cout << '.';
	}

	for (int i = 0; i < 200; i++) {
		SCOPED_TRACE("test_SDA2_rnd");
		test_SDA2_rnd();
		if (i % 40 == 0) cout << '.';
	}
	cout << endl;
}

BitArray randbit(unsigned int len, unsigned int & cnt) {
	BitArray b = BitArrayBuilder::create(len);
	cnt = 0;
	for (size_t i = 0; i < len; ++i)
		if (rand() % 100 < 10) { b.setbit(i, true); cnt++; } else b.setbit(i, false);
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
	for (uint64_t i = 0; i < N; ++i) {
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
	for (uint64_t i = 0; i < M; ++i) {
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
	for (uint64_t i = 0; i < N; ++i) {
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
	for (uint64_t i = 0; i < M; ++i) {
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


TEST(sdatest_sml, sda2_inc) {
	SDArraySmlBuilder bd;
	uint64_t N = 1000;
	vector<uint64_t> vals(N);
	vector<uint64_t> cums(N+1);
	uint64_t sum = 0;
	for (uint64_t i = 0; i < N; ++i) {
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
	for (uint64_t i = 0; i < M; ++i) {
		uint64_t val = rand() % sum;

		vector<uint64_t>::iterator it = lower_bound(cums.begin(), cums.end(), val);
		size_t ind = it - cums.begin();

		if (ind < cums.size())
			ASSERT(val <= cums[ind]);
		ASSERT_EQ(ind, sda.rank(val));
	}
}


TEST(sdatest_sml, sda2_ones) {
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
	for (int i = 0; i < N; ++i) {
		if (i != sda.rank(i)) {
			auto x = sda.rank(i);
			ASSERT_EQ(i, sda.rank(i));
		}
	}

}

TEST(sdatest_sml, sda2_zeros) {
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
		v = BitArrayBuilder::create(vec.size());
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
}

void test_rank3(int len) {
	std::vector<bool> vec;
	for (int i = 0; i < len; ++i) {
		if (rand() % 20 == 1)
			vec.push_back(true);
		else vec.push_back(false);
	}

	BitArray v;
	v = BitArrayBuilder::create(vec.size());
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
}

TEST(sdatest_sml, rank4) {
	const size_t len = 160;
	int inp[len] = {1, 18, 25, 32, 56, 63, 70, 105, 108, 139, 147, 151, 153, 156, 163, 177, 184, 195, 196, 200, 202, 225, 256, 272, 274, 303, 311, 326, 338, 345, 348,
		358, 361, 374, 380, 387, 420, 421, 433, 467, 489, 492, 526, 532, 550, 559, 561, 571, 579, 594, 600, 615, 624, 685, 710, 715, 729, 744, 747, 796,
		835, 842, 856, 858, 908, 913, 920, 927, 932, 937, 942, 948, 965, 999, 1008, 1018, 1040, 1057, 1071, 1098, 1125, 1146, 1150, 1168, 1195, 1210,
		1214, 1230, 1237, 1240, 1248, 1256, 1268, 1287, 1289, 1299, 1301, 1316, 1325, 1327, 1338, 1349, 1363, 1387, 1417, 1421, 1429, 1443, 1453,
		1464, 1470, 1477, 1490, 1492, 1518, 1532, 1548, 1565, 1585, 1592, 1595, 1604, 1622, 1624, 1649, 1654, 1666, 1670, 1680, 1689, 1692, 1703,
		1705, 1724, 1734, 1743, 1765, 1767, 1776, 1778, 1781, 1794, 1800, 1818, 1826, 1839, 1856, 1864, 1874, 1878, 1886, 1892, 1932, 1944, 1953,
		1967, 1983, 1984, 1992, 1995};
	SDRankSelectBuilderSml bd;
	std::vector<bool> vec;
	vec.resize(inp[len-1]+1, false);
	for (size_t i = 0; i < len; i++) {
		bd.add_inc(inp[i]);
		vec[inp[i]] = true;
	}
	vector<int> ranks(vec.size() + 1);
	ranks[0] = 0;
	for (unsigned int i = 1; i <= vec.size(); i++)
		if (vec[i-1]) ranks[i] = ranks[i-1] + 1;
		else ranks[i] = ranks[i-1];
		SDRankSelectSml r;
		bd.build(&r);

		for (int i = 0; i <= vec.size(); ++i)
			if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT(ranks[i] == r.rank(i));
			}
		for (unsigned int i = 0; i < len; ++i) {
			ASSERT_EQ(inp[i], r.select(i));
		}
}

TEST(sdatest_sml, bug2) {
	const int len = 495;
	int inp[len] = {0, 1, 6, 8, 9, 12, 14, 15, 17, 18, 19, 20, 24, 25, 28, 31, 32, 35, 36, 37, 38, 39, 41, 46, 49, 50,
		51, 52, 53, 54, 58, 59, 60, 61, 62, 64, 68, 70, 71, 72, 73, 76, 79, 80, 81, 82, 84, 85, 86, 87, 89, 91,
		92, 93, 94, 95, 97, 98, 99, 100, 102, 104, 105, 106, 108, 109, 110, 113, 114, 116, 117, 118, 119, 120
		, 121, 122, 125, 126, 127, 128, 133, 134, 136, 139, 140, 142, 143, 144, 145, 146, 147, 148, 149, 150
		, 153, 155, 157, 158, 161, 162, 164, 165, 166, 167, 170, 171, 172, 173, 174, 175, 179, 180, 181, 184
		, 185, 187, 188, 189, 191, 193, 196, 197, 198, 199, 200, 201, 202, 204, 207, 209, 211, 212, 213, 214
		, 215, 217, 219, 220, 222, 224, 225, 226, 227, 228, 229, 230, 232, 233, 234, 236, 237, 238, 241, 242
		, 244, 245, 246, 248, 253, 254, 255, 256, 258, 262, 264, 266, 269, 271, 276, 277, 278, 282, 287, 288
		, 289, 290, 291, 292, 293, 296, 297, 298, 299, 301, 305, 307, 308, 309, 312, 313, 314, 315, 317, 319
		, 321, 323, 325, 326, 327, 328, 330, 331, 332, 340, 341, 343, 345, 346, 347, 349, 352, 353, 354, 355
		, 357, 358, 359, 361, 363, 365, 366, 367, 368, 369, 370, 371, 374, 376, 378, 379, 382, 383, 385, 386
		, 387, 389, 390, 392, 393, 395, 396, 398, 400, 401, 402, 403, 406, 408, 409, 411, 412, 413, 414, 416
		, 417, 418, 419, 420, 421, 423, 424, 430, 431, 437, 438, 439, 440, 443, 444, 445, 446, 448, 450, 451
		, 452, 453, 455, 456, 457, 458, 459, 460, 461, 462, 464, 465, 466, 468, 469, 470, 472, 473, 474, 475
		, 476, 477, 478, 480, 481, 483, 485, 486, 491, 492, 493, 495, 496, 497, 498, 499, 500, 501, 502, 503
		, 504, 505, 507, 508, 509, 510, 512, 514, 515, 517, 518, 520, 521, 523, 524, 525, 526, 527, 528, 530
		, 532, 534, 539, 540, 541, 542, 543, 544, 545, 546, 548, 550, 553, 554, 557, 561, 562, 563, 564, 565
		, 566, 567, 569, 571, 572, 573, 574, 576, 579, 580, 581, 582, 583, 585, 587, 588, 589, 590, 591, 592
		, 593, 594, 596, 597, 598, 599, 600, 601, 602, 604, 607, 608, 610, 611, 612, 616, 617, 619, 620, 621
		, 623, 624, 625, 626, 627, 628, 630, 631, 633, 634, 635, 637, 638, 639, 640, 641, 642, 643, 644, 645
		, 649, 650, 651, 653, 654, 656, 658, 659, 660, 662, 663, 667, 669, 671, 672, 673, 674, 675, 677, 678
		, 680, 682, 683, 685, 686, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 699, 700, 701, 702, 703
		, 704, 705, 706, 707, 708, 709, 710, 711, 714, 715, 717, 719, 723, 725, 726, 728, 729, 731, 732, 733
		, 734, 736, 738, 739, 740, 742, 743, 745, 747, 748, 749, 750, 751, 755, 757, 759, 760, 762, 763, 765
		, 768};
	SDRankSelectBuilderSml bd;
	std::vector<bool> vec;
	vec.resize(inp[len-1]+1, false);
	for (size_t i = 0; i < len; i++) {
		bd.add_inc(inp[i]);
		vec[inp[i]] = true;
	}
	vector<int> ranks(vec.size() + 1);
	ranks[0] = 0;
	for (unsigned int i = 1; i <= vec.size(); i++)
		if (vec[i-1]) ranks[i] = ranks[i-1] + 1;
		else ranks[i] = ranks[i-1];
		SDRankSelectSml r;
		bd.build(&r);

		for (int i = 0; i <= vec.size(); ++i)
			if (ranks[i] != r.rank(i)) {
			cout << "rank " << i << " " << ranks[i] << " " << r.rank(i) << endl;
			ASSERT(ranks[i] == r.rank(i));
			}
		for (unsigned int i = 0; i < len; ++i) {
			ASSERT_EQ(inp[i], r.select(i));
		}
}

TEST(sdatest_sml, test_sda2_rnd_all) {
	for (int i = 0; i < 500; i++) {
		test_rank3(1020 + rand() % 8);
		if (i % 10 == 0) cout << '.';
	}
	for (int i = 0; i < 500; i++) {
		test_rank2(1020 + rand() % 8);
		if (i % 10 == 0) cout << '.';
	}
	for (int i = 0; i < 200; i++) {
		test_sda2_rand();
		test_sda2_rand2();
		if (i % 10 == 0) cout << '.';
	}
	cout << endl;
}

TEST(sdatest_sml, testrnd2) {
	std::vector<unsigned> vec;
	vec = gen_zeros();
	check_prefixsum_lookup<SDArraySml>(vec);
	vec = gen_ones();
	check_prefixsum_lookup<SDArraySml>(vec);
	vec = gen_same(2);
	check_prefixsum_lookup<SDArraySml>(vec);
	vec = gen_same(3);
	check_prefixsum_lookup<SDArraySml>(vec);

	vec = gen_increasing();
	check_prefixsum_lookup<SDArraySml>(vec);
	for (unsigned i = 0; i < 10; ++i) {
		vec = gen_rand();
		check_prefixsum_lookup<SDArraySml>(vec);
		vec = gen_rand2();
		check_prefixsum_lookup<SDArraySml>(vec);
	}
}

TEST(sda_compress, test1) {
	std::vector<unsigned> vec;
	vec = gen_zeros();
	check_prefixsum_lookup<SDArrayCompress>(vec);
	vec = gen_ones();
	check_prefixsum_lookup<SDArrayCompress>(vec);
	vec = gen_same(2);
	check_prefixsum_lookup<SDArrayCompress>(vec);
	vec = gen_same(3);
	check_prefixsum_lookup<SDArrayCompress>(vec);

	vec = gen_increasing();
	check_prefixsum_lookup<SDArrayCompress>(vec);
	for (unsigned i = 0; i < 10; ++i) {
		vec = gen_rand();
		check_prefixsum_lookup<SDArrayCompress>(vec);
		vec = gen_rand2();
		check_prefixsum_lookup<SDArrayCompress>(vec);
	}
}

TEST(sda_rl, test1) {
	std::vector<unsigned> vec;
	vec = gen_zeros();
	check_prefixsum_lookup<SDArrayRunLen>(vec);
	vec = gen_ones();
	check_prefixsum_lookup<SDArrayRunLen>(vec);
	vec = gen_same(2);
	check_prefixsum_lookup<SDArrayRunLen>(vec);
	vec = gen_same(3);
	check_prefixsum_lookup<SDArrayRunLen>(vec);

	vec = gen_increasing();
	check_prefixsum_lookup<SDArrayRunLen>(vec);
	for (unsigned i = 0; i < 10; ++i) {
		vec = gen_rand();
		check_prefixsum_lookup<SDArrayRunLen>(vec);
		vec = gen_rand2();
		check_prefixsum_lookup<SDArrayRunLen>(vec);
	}
}


TEST(sda_adp_rl, test1) {
	std::vector<unsigned> vec;
	vec = gen_zeros();
	check_prefixsum_lookup<SDArrayCRL>(vec);
	vec = gen_ones();
	check_prefixsum_lookup<SDArrayCRL>(vec);
	vec = gen_same(2);
	check_prefixsum_lookup<SDArrayCRL>(vec);
	vec = gen_same(3);
	check_prefixsum_lookup<SDArrayCRL>(vec);

	vec = gen_increasing();
	check_prefixsum_lookup<SDArrayCRL>(vec);
	for (unsigned i = 0; i < 10; ++i) {
		vec = gen_rand();
		check_prefixsum_lookup<SDArrayCRL>(vec);
		vec = gen_rand2();
		check_prefixsum_lookup<SDArrayCRL>(vec);
	}
}


}//namespace
