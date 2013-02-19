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
unsigned int RunLenSumArray5::pslen(unsigned int i) const { return 0; throw "wrong"; }
unsigned int RunLenSumArray5::range_value(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i / VALUE_GROUP;
	mscds::EnumeratorInt<uint64_t> * g = vals.getEnum(p*VALUE_GROUP);
	unsigned int x;
	for (size_t i = 0; i < r; ++i) g->next();
	x = g->next();
	delete g;
	return x;
}

uint64_t RunLenSumArray5::range_psum(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i / VALUE_GROUP;
	uint64_t cpsum = psum.prefixsum(p+1);
	if (r == 0) return cpsum;
	mscds::EnumeratorInt<uint64_t> * g = vals.getEnum(p*VALUE_GROUP);
	for (size_t j = 0; j < r; ++j)
		cpsum += g->next() * range_len(p*VALUE_GROUP + j);
	delete g;
	return cpsum;
}


unsigned int RunLenSumArray5::count_range(unsigned int pos) const {
	return 0;
}

uint64_t RunLenSumArray5::sum(uint32_t pos) const {
	uint64_t p = itv.find_interval(pos+1).second; //start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = itv.int_start(p);
	if (sp == pos)
		return range_psum(p);
	assert(sp < pos);
	uint32_t kl = pos - sp;
	uint32_t rangelen = itv.int_len(p);
	if (rangelen >= kl) {
		uint64_t pm, val;
		pm = range_psum(p);
		val = range_value(p);
		return  pm + kl*(val);
	} else
		return range_psum(p+1);
}

int64_t RunLenSumArray5::sum_delta(uint32_t pos, int64_t delta) const {
	uint64_t p = itv.find_interval(pos+1).second;
	if (p == 0) return 0;
	p--;
	uint64_t sp = itv.int_start(p);
	if (sp == pos)
		return (int64_t)range_psum(p) + delta * pslen(p);
	assert(sp < pos);
	uint32_t kl = pos - sp;
	uint64_t ps;
	uint32_t rangelen = 0;//rlen.lookup(p, ps);
	if (rangelen >= kl) {
		uint64_t pm, val;
		pm = range_psum(p);
		val = range_value(p);
		return  pm + kl*(val) + delta * (kl + ps);
	} else
		return range_psum(p+1) + delta*(rangelen + ps);
}

unsigned int RunLenSumArray5::countnz(unsigned int pos) const {
	uint64_t p = itv.find_interval(pos+1).second;
	if (p == 0) return 0;
	p--;
	uint64_t sp = itv.int_start(p);
	assert(sp <= pos);
	uint64_t ps = 0;
	uint32_t rangelen = 0;//rlen.lookup(p, ps);
	return std::min<uint32_t>((pos - sp), rangelen) + ps;
}

unsigned int RunLenSumArray5::access(unsigned int pos) const {
	uint64_t p = itv.find_interval(pos+1).second;
	if (p == 0) return 0;
	p--;
	uint64_t sp = itv.int_start(p); //start.select(p);
	assert(sp <= pos);
	uint32_t rangelen = 0;//rlen.lookup(p);
	if (rangelen > (pos - sp)) {
		return range_value(p);
	} else
		return 0;
}

int RunLenSumArray5::prev(unsigned int pos) const {
	uint64_t p = itv.find_interval(pos+1).second;
	if (p == 0) return -1;
	p--;
	uint64_t sp = itv.int_start(p);
	assert(sp <= pos);
	uint32_t rangelen = 0;//rlen.lookup(p);
	if (rangelen > (pos - sp))
		return  pos;
	else return sp + rangelen - 1;
}

int RunLenSumArray5::next(unsigned int pos) const {
	uint64_t p = itv.find_interval(pos+1).second;
	if (p == 0) return (len > 0) ? itv.int_start(0) : -1;
	p--;
	uint64_t sp = itv.int_start(p);
	assert(sp <= pos);
	uint32_t rangelen = 0;//rlen.lookup(p);
	if (rangelen > (pos - sp))
		return pos;
	else {
		if (p == len-1) return -1;
		else return itv.int_start(p+1);
	}
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