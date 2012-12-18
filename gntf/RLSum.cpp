#include "RLSum.h"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace mscds {


void RunLenSumArrayBuilder::clear() {
	len = 0;
	lastst = 0;
	stbd.clear();
	psbd.clear();
	rlenbd.clear();
	stpos.clear();
}

void RunLenSumArrayBuilder::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (st < lastst) throw std::logic_error("required sorted array");
	psbd.add(llen * v);
	stpos.push_back(st);
	rlenbd.add(llen);
	lastst = st;
}

void RunLenSumArrayBuilder::build(RunLenSumArray *out) {
	out->clear();
	out->len = len;
	out->start.build(stpos);
	psbd.build(&(out->psum));
	rlenbd.build(&(out->rlen));
	clear();
}

void RunLenSumArrayBuilder::build(OArchive& ar) {
	ar.startclass("run_length_sum_array", 1);
	ar.var("len").save(len);
	stbd.build(stpos);
	stbd.save(ar.var("start"));
	psbd.build(ar.var("prefixsum"));
	rlenbd.build(ar.var("range_len"));
	ar.endclass();
	clear();
}

//-----------------------------------------------------------------------------

void RunLenSumArray::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array", 1);
	ar.var("len").save(len);
	start.save(ar.var("start"));
	psum.save(ar.var("prefixsum"));
	rlen.save(ar.var("range_len"));
	ar.endclass();
}

void RunLenSumArray::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array");
	ar.var("len").load(len);
	start.load(ar.var("start"));
	psum.load(ar.var("prefixsum"));
	rlen.load(ar.var("range_len"));
	ar.endclass();
}

uint64_t RunLenSumArray::range_start(unsigned int i) { return start.select(i); }
uint64_t RunLenSumArray::range_psum(unsigned int i) { return psum.prefixsum(i); }
unsigned int RunLenSumArray::range_len(unsigned int i) { return rlen.prefixsum(i+1) - rlen.prefixsum(i); }

RunLenSumArray::RunLenSumArray(): len(0) {}

uint64_t RunLenSumArray::sum(uint32_t pos) {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	if (sp == pos)
		return psum.prefixsum(p);
	assert(sp < pos);
	uint32_t kl = pos - sp;
	//uint32_t rangelen = rlen.prefixsum(p+1) - rlen.prefixsum(p);
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen >= kl) {
		//uint64_t pm = psum.prefixsum(p);
		//return  pm + kl*((psum.prefixsum(p+1) - pm)/rangelen);
		uint64_t pm = 0, val;
		val = psum.lookup(p, pm);
		return  pm + kl*(val/rangelen);
	} else
		return psum.prefixsum(p+1);
}

unsigned int RunLenSumArray::length() const {
	return len;
}

void RunLenSumArray::clear() {
	len = 0;
	start.clear();
	psum.clear();
	rlen.clear();
}

}//namespace