#include "RLSum4.h"



#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace app_ds {
using namespace mscds;


void RunLenSumArrayBuilder4::clear() {
	len = 0;
	lastst = 0;
	stbd.clear();
	rlbd.clear();
	psbd.clear();
	spsbd.clear();
	psum = 0;
	lastv = 0;
	cnt = 0;
}

const unsigned int VALUE_GROUP = 64;

void RunLenSumArrayBuilder4::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (llen == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	//psbd.add(llen * v);
	stbd.add_inc(st);
	rlbd.add(llen);
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

void RunLenSumArrayBuilder4::build(RunLenSumArray4 *out) {
	out->clear();
	out->len = len;
	stbd.build(&(out->start));

	rlbd.build(&(out->rlen));
	psbd.build(&(out->psum));
	spsbd.build(&(out->sqrsum));
	vals.build(&(out->vals));
	clear();
}

void RunLenSumArrayBuilder4::build(OArchive& ar) {
	RunLenSumArray4 a;
	build(&a);
	a.save(ar);
}

//-----------------------------------------------------------------------------

void RunLenSumArray4::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array3", 1);
	ar.var("len").save(len);
	start.save(ar.var("start"));
	rlen.save(ar.var("rlen"));
	vals.save(ar.var("vals"));
	psum.save(ar.var("sampled_psum"));
	sqrsum.save(ar.var("sampled_sqrsum"));
	ar.endclass();
}

void RunLenSumArray4::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array3");
	ar.var("len").load(len);
	start.load(ar.var("start"));
	rlen.load(ar.var("rlen"));
	vals.load(ar.var("vals"));
	psum.load(ar.var("sampled_psum"));
	sqrsum.load(ar.var("sampled_sqrsum"));
	ar.endclass();
}

unsigned int RunLenSumArray4::range_start(unsigned int i) const { return start.select(i); }
unsigned int RunLenSumArray4::range_len(unsigned int i) const { return rlen.lookup(i); }
unsigned int RunLenSumArray4::pslen(unsigned int i) const { return rlen.prefixsum(i+1); }
unsigned int RunLenSumArray4::range_value(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i / VALUE_GROUP;
	mscds::EnumeratorInt<uint64_t> * g = vals.getEnum(p*VALUE_GROUP);
	unsigned int x;
	for (size_t i = 0; i < r; ++i) g->next();
	x = g->next();
	delete g;
	return x;
}

uint64_t RunLenSumArray4::range_psum(unsigned int i) const {
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


unsigned int RunLenSumArray4::count_range(unsigned int pos) const {
	return 0;
}

uint64_t RunLenSumArray4::sum(uint32_t pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	if (sp == pos)
		return range_psum(p);
	assert(sp < pos);
	uint32_t kl = pos - sp;
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen >= kl) {
		uint64_t pm, val;
		pm = range_psum(p);
		val = range_value(p);
		return  pm + kl*(val);
	} else
		return range_psum(p+1);
}

int64_t RunLenSumArray4::sum_delta(uint32_t pos, int64_t delta) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	if (sp == pos)
		return (int64_t)range_psum(p) + delta * rlen.prefixsum(p);
	assert(sp < pos);
	uint32_t kl = pos - sp;
	uint64_t ps;
	uint32_t rangelen = rlen.lookup(p, ps);
	if (rangelen >= kl) {
		uint64_t pm, val;
		pm = range_psum(p);
		val = range_value(p);
		return  pm + kl*(val) + delta * (kl + ps);
	} else
		return range_psum(p+1) + delta*(rangelen + ps);
}

unsigned int RunLenSumArray4::countnz(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint64_t ps = 0;
	uint32_t rangelen = rlen.lookup(p, ps);
	return std::min<uint32_t>((pos - sp), rangelen) + ps;
}

unsigned int RunLenSumArray4::access(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen > (pos - sp)) {
		return range_value(p);
	} else
		return 0;
}

int RunLenSumArray4::prev(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return -1;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen > (pos - sp))
		return  pos;
	else return sp + rangelen - 1;
}

int RunLenSumArray4::next(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return (len > 0) ? start.select(0) : -1;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen > (pos - sp))
		return pos;
	else {
		if (p == len-1) return -1;
		else return start.select(p+1);
	}
}

unsigned int RunLenSumArray4::length() const {
	return len;
}

void RunLenSumArray4::clear() {
	len = 0;
	start.clear();
	rlen.clear();
	vals.clear();
}

}//namespace