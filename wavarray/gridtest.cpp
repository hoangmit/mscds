
#include "wat_array.h"
#include "utils/utest.h"
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;
using namespace mscds;

TEST(watarr, watarr_0) {
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
	ASSERT(5 == arr.rank(1, 8));
}

TEST(watarr, wat_1) {
	unsigned int arr[8] = {2, 7, 1, 7, 3, 0, 4, 4};
	vector<uint64_t> v(arr, arr+8);
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);
	ASSERT_EQ("01010011" "10101100" "10010011", wq.bit_layers().to_str());
	for (int i = 0; i < 8; ++i)
		ASSERT_EQ(v[i], wq.access(i));

	ASSERT_EQ(0, wq.select(2, 0));
	ASSERT_EQ(1, wq.select(7, 0));
	ASSERT_EQ(2, wq.select(1, 0));
	ASSERT_EQ(3, wq.select(7, 1));
	ASSERT_EQ(4, wq.select(3, 0));
	ASSERT_EQ(5, wq.select(0, 0));
	ASSERT_EQ(6, wq.select(4, 0));
	ASSERT_EQ(7, wq.select(4, 1));
}

TEST(watarr, wat_2){
	vector<uint64_t> v;
	const int range = 8;
	const int len = 100;
	for (int i = 0; i < len; ++i)
		v.push_back(rand() % range);

	vector<int> Cnt(range,0);
	for (int i = 0; i < len; ++i)
		Cnt[v[i]]++;

	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < len; ++i) {
		unsigned int testv = rand() % (len / 10);
		int count = 0;
		for (int j = 0; j < i; ++j)
			if (v[j] < testv) count++;

		unsigned int rv = wq.rankLessThan(testv, i);
		ASSERT(count == rv);
		ASSERT(v[i] == wq.access(i));
		unsigned int rx = wq.rank(v[i], i);
		ASSERT(i == wq.select(v[i], rx));

	}
}

#include "bitarray/rrr.h"

TEST(watarr, wat_3){
	
	vector<uint64_t> v;
	for (unsigned int i = 0; i < 9; i++)
		v.push_back(i + 1);
	typedef WatQueryGen<RRR> WatQuery2;
	typedef WatBuilderGen<RRR> WatBuilder2;

	WatQuery2 wq;
	WatBuilder2 bd;
	bd.build(v, &wq);

	ASSERT_EQ(1, wq.rank(6, 8));
}



TEST(watarr, wat_access) {
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
		ASSERT(count == rv);
		ASSERT(v[i] == wq.access(i));
		unsigned int rx = wq.rank(v[i], i);
		ASSERT(i == wq.select(v[i], rx));
	}
}

TEST(watarr, rank_access_bigrange) {
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
		ASSERT_EQ(count, rv);
		ASSERT_EQ(v[i], wq.access(i));
		unsigned int rx = wq.rank(v[i], i);
		ASSERT(i == wq.select(v[i], rx));
	}
}

TEST(watarr, rank_access_smallrange) {
	vector<uint64_t> v;
	int len = 1000;
	for (int i = 0; i < len; ++i)
		v.push_back(rand() % 18);
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < len; ++i) {
		unsigned int testv = rand() % 18;
		int count = 0;
		for (int j = 0; j < i; ++j)
			if (v[j] < testv) count++;
		unsigned int rv = wq.rankLessThan(testv, i);
		ASSERT_EQ(count, rv);
		ASSERT_EQ(v[i], wq.access(i));
		unsigned int rx = wq.rank(v[i], i);
		ASSERT(i == wq.select(v[i], rx));

	}
}

TEST(watarr, select_rank) {
	vector<uint64_t> v;
	int len = 1000, maxval=18;
	vector<vector<unsigned int> > pos(maxval);
	for (int i = 0; i < len; ++i) {
		unsigned int val = rand() %  maxval;
		v.push_back(val);
		pos[val].push_back(i);
	}
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < maxval; ++i) {
		for (int j = 0; j < pos[i].size(); ++j) {
			ASSERT_EQ(pos[i][j], wq.select(i, j));
			ASSERT_EQ(j, wq.rank(i, wq.select(i, j)));
		}
	}
}

TEST(watarr, minmax_handmade_test) {
	//WatBuilder wb;
	uint64_t arr[8] = {2, 7, 1, 7, 3, 0, 4, 4};
	vector<uint64_t> v;
	for (int i = 0; i < 8; ++i)
		v.push_back(arr[i]);
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < 8; ++i)
		for (int j = i+1; j < 8; ++j) {
			unsigned int maxv, minv;
			maxv = *std::max_element(arr + i, arr + j);
			minv = *std::min_element(arr + i, arr + j);
			uint64_t pos;
			ASSERT_EQ(maxv, wq.maxValue(i, j, pos));
			ASSERT_EQ(minv, wq.minValue(i, j, pos));
		}
}

bool push_vec_ppii(void * context, uint64_t c, uint64_t pos) {
	vector<pair<int, int> > * v = (vector<pair<int, int> > *) context;
	v->push_back(std::make_pair<int, int>((int)c, (int)pos));
	return false;
}

void manuallist(int min_c, int max_c, int beg_pos, int end_pos, const vector<uint64_t>& input, std::vector<pair<int, int> >& results) {
	results.clear();
	for (int i = beg_pos; i < end_pos; ++i) {
		if (input[i] >= min_c && input[i] < max_c) {
			results.push_back(make_pair(input[i], i));
		}
	}
}

TEST(watarr, listing) {
	vector<uint64_t> v;
	int len = 50, maxval=18;
	for (int i = 0; i < len; ++i) {
		unsigned int val = rand() %  maxval;
		v.push_back(val);
	}
	WatQuery wq;
	WatBuilder bd;
	bd.build(v, &wq);

	for (int i = 0; i < len; ++i) {
		for (int j = i; j <= len; ++j) 
			for (int v1 = 0; v1 < maxval; ++v1)
				for (int v2 = v1; v2 <= maxval; ++v2) {
					vector<pair<int, int> > exp;
					manuallist(v1, v2, i, j, v, exp);
					vector<pair<int, int> > rs;
					wq.list_each(v1, v2, i, j, push_vec_ppii, &rs);
					ASSERT_EQ(exp.size(), rs.size());
					if (exp.size() > 0) {
						std::sort(exp.begin(), exp.end());
						std::sort(rs.begin(), rs.end());
						for (int k = 0; k < exp.size(); ++k) {
							//ASSERT_EQ(exp[k].first, rs[k].first);
							//ASSERT_EQ(exp[k].second, rs[k].second);
							ASSERT_EQ(exp[k], rs[k]);
						}
					}
				}
	}
}

//------------------------------------------------------------------------------

TEST(grid, gridquerytest_x) {
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
	vector<unsigned int>  results;
	gq.process(&arr, X, Y, &results);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			cout << results[i*9+j] << "  ";
		}
		cout << endl;
	}
}

TEST(grid, querytest1) {
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
	vector<unsigned int>  results;
	gq.process(&arr, X, Y, &results);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			cout << results[i*2+j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
}


TEST(grid, querytest2) {
	vector<uint64_t> inp;
	inp.push_back(1);
	inp.push_back(2);
	WatQuery arr;
	WatBuilder bd;
	bd.build(inp, &arr);

	uint64_t llt, lmt, r;
	arr.rankAll(2, 1, r, llt, lmt);
	ASSERT(1 == llt);
	arr.rankAll(2, 2, r, llt, lmt);
	ASSERT(1 == llt);
	arr.rankAll(2, 1, r, llt, lmt);
	ASSERT(1 == llt+r);
	arr.rankAll(2, 2, r, llt, lmt);
	ASSERT(2 == llt+r);

	GridQuery gq;
	vector<unsigned int> X, Y;
	X.push_back(1);
	X.push_back(2);
	Y.push_back(2);
	Y.push_back(3);

	vector<unsigned int>  results;
	gq.process(&arr, X, Y, &results);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			cout << results[i*2+j] << "  ";
		}
		cout << endl;
	}
	cout << endl;
}


