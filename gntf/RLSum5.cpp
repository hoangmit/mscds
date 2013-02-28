#include "RLSum5.h"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace app_ds {
using namespace mscds;


void RunLenSumArrayBuilder5::clear() {
	len = 0;
	lastst = 0;
	itvb.clear();
	psbd.clear();
	spsbd.clear();
	psum = 0;
	lastv = 0;
	cnt = 0;
}

const unsigned int VALUE_GROUP = 64;

void RunLenSumArrayBuilder5::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (llen == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	//psbd.add(llen * v);
	itvb.add(st, ed);
	lastst = st;
	if (cnt % VALUE_GROUP == 0) {
		psbd.add_inc(psum);
		spsbd.add_inc(sqpsum);
		//
	}
	//
	vals.add(v);
	lastv = v;

	psum += llen * v;
	sqpsum += llen * (v*v);
	cnt++;
}

void RunLenSumArrayBuilder5::build(RunLenSumArray5 *out) {
	out->clear();
	out->len = len;
	itvb.build(&(out->itv));
	psbd.build(&(out->psum));
	spsbd.build(&(out->sqrsum));
	vals.build(&(out->vals));
	clear();
}

void RunLenSumArrayBuilder5::build(OArchive& ar) {
	RunLenSumArray5 a;
	build(&a);
	a.save(ar);
}

//-----------------------------------------------------------------------------

void RunLenSumArray5::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array3", 1);
	ar.var("len").save(len);
	itv.save(ar.var("intervals"));

	vals.save(ar.var("vals"));
	psum.save(ar.var("sampled_psum"));
	sqrsum.save(ar.var("sampled_sqrsum"));
	ar.endclass();
}

void RunLenSumArray5::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array3");
	ar.var("len").load(len);
	itv.load(ar.var("intervals"));
	vals.load(ar.var("vals"));
	psum.load(ar.var("sampled_psum"));
	sqrsum.load(ar.var("sampled_sqrsum"));
	ar.endclass();
}

unsigned int RunLenSumArray5::range_start(unsigned int i) const { return itv.int_start(i); }
unsigned int RunLenSumArray5::range_len(unsigned int i) const { return itv.int_len(i); }

unsigned int RunLenSumArray5::range_value(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i / VALUE_GROUP;
	PRSumArr::Enumerator g;
	vals.getEnum(p*VALUE_GROUP, &g);
	unsigned int x;
	for (size_t i = 0; i < r; ++i) g.next();
	x = g.next();
	return x;
}

unsigned int RunLenSumArray5::count_range(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.first == 0 && res.second == 0) return 0;
}

uint64_t RunLenSumArray5::range_psum(unsigned int i) const {
	return sum(range_start(i));
}

uint64_t RunLenSumArray5::sum(uint32_t pos) const {
	if (pos == 0) return 0;
	auto res = itv.find_cover(pos - 1);
	if (res.first == 0 && res.second == 0) return 0;
	size_t r = res.first % VALUE_GROUP;
	size_t p = res.first / VALUE_GROUP;
	uint64_t cpsum = psum.prefixsum(p+1);
	PRSumArr::Enumerator g;
	if (r > 0 || res.second > 0) {
		vals.getEnum(p*VALUE_GROUP, &g);
		for (size_t j = 0; j < r; ++j)
			cpsum += g.next() * range_len(p*VALUE_GROUP + j);
	} 
	if (res.second > 0) return cpsum + g.next() * res.second;
	else return cpsum;
}

int64_t RunLenSumArray5::sum_delta(uint32_t pos, int64_t delta) const {
	return delta * itv.coverage(pos) + sum(pos);
}

unsigned int RunLenSumArray5::countnz(unsigned int pos) const {
	return itv.coverage(pos);
}

unsigned int RunLenSumArray5::access(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second != 0) return range_value(res.first);
	else return 0;
}

int RunLenSumArray5::prev(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second > 0) return pos;
	else if (res.first > 0) return itv.int_end(res.first - 1) - 1;
	else return -1;
}

int RunLenSumArray5::next(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second > 0) return pos;
	else if (res.first < len) return itv.int_start(res.first);
	else return -1;
}

unsigned int RunLenSumArray5::length() const {
	return len;
}

void RunLenSumArray5::clear() {
	len = 0;
	itv.clear();
	vals.clear();
}

}//namespace