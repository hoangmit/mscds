
#include "utils/utest.h"
#include "nintv_fuse.h"
#include "intv_rand_gen.hpp"
using namespace mscds;
using namespace std;
using namespace app_ds;


template<typename T>
void test_intervals_easy(const std::vector<std::pair<unsigned int, unsigned int> >& rng, const T& r, int testid = 0) {
	ASSERT_EQ(rng.size(), r.length());
	size_t pslen = 0;
	for (size_t i = 0; i < rng.size(); ++i) {
		ASSERT_EQ(rng[i].first, r.int_start(i));
		ASSERT_EQ(rng[i].second - rng[i].first, r.int_len(i)) << "i = " << i << std::endl;
		ASSERT_EQ(rng[i].second, r.int_end(i));
		ASSERT_EQ(pslen, r.int_psrlen(i));
		pslen += rng[i].second - rng[i].first;
		auto px = r.int_startend(i);
		ASSERT(rng[i].first == px.first && rng[i].second == px.second);
	}
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

}

int main() {
	test1();
	//test2();
	return 0; 
}

