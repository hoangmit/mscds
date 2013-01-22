#include "RLSum3.h"



#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace app_ds {
using namespace mscds;


void RunLenSumArrayBuilder3::clear() {
	len = 0;
	lastst = 0;
	stbd.clear();
	rlbd.clear();
	vpt.clear();
	psbd.clear();
	spsbd.clear();
	psum = 0;
	lastv = 0;
	cnt++;
}

const unsigned int VALUE_GROUP = 64;

void RunLenSumArrayBuilder3::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (llen == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	//psbd.add(llen * v);
	stbd.add_inc(st);
	rlbd.add(llen);
	lastst = st;
	if (cnt % VALUE_GROUP == 0) {
		vpt.add_inc(enc.length());
		psbd.add_inc(psum);
		spsbd.add_inc(sqpsum);
		//
		coder::CodePr c = dc.encode(v);
		enc.puts(c.first, c.second);
	}else {
		uint64_t vx = ((int64_t)v) - lastv;
		coder::CodePr c = dc.encode(coder::absmap(vx)+1);
		enc.puts(c.first, c.second);
	}
	//
	lastv = v;

	psum += llen * v;
	sqpsum += llen * (v*v);
	cnt++;
}

void RunLenSumArrayBuilder3::build(RunLenSumArray3 *out) {
	out->clear();
	enc.close();
	out->len = len;
	stbd.build(&(out->start));

	rlbd.build(&(out->rlen));
	vpt.build(&(out->ptr));
	psbd.build(&(out->psumx));
	spsbd.build(&(out->sqrsum));
	out->enc = BitArray::create(enc.data_ptr(), enc.length());
	clear();
}

void RunLenSumArrayBuilder3::build(OArchive& ar) {
	RunLenSumArray3 a;
	build(&a);
	a.save(ar);
}

//-----------------------------------------------------------------------------

void RunLenSumArray3::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array3", 1);
	ar.var("len").save(len);
	start.save(ar.var("start"));
	rlen.save(ar.var("rlen"));
	ptr.save(ar.var("ptr"));
	enc.save(ar.var("enc"));
	ar.endclass();
}

void RunLenSumArray3::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array3");
	ar.var("len").load(len);
	start.load(ar.var("start"));
	rlen.load(ar.var("rlen"));
	ptr.load(ar.var("ptr"));
	enc.load(ar.var("enc"));
	ar.endclass();
}

unsigned int RunLenSumArray3::range_start(unsigned int i) const { return start.select(i); }
unsigned int RunLenSumArray3::range_len(unsigned int i) const { return rlen.lookup(i); }
unsigned int RunLenSumArray3::pslen(unsigned int i) const { return rlen.prefixsum(i+1); }
unsigned int RunLenSumArray3::range_value(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i/VALUE_GROUP;
	if (r == 0) return psumx.prefixsum(p);
	size_t pos = ptr.prefixsum(p);
	mscds::IWBitStream is(enc.data_ptr(), enc.length(), pos);
	coder::DeltaCoder dc;
	coder::CodePr c;
	for (unsigned int j = 0; j < r; ++j) {
		c = dc.decode2(is.peek());
		is.skipw(c.second);
	}
	return c.first; 
}

uint64_t RunLenSumArray3::range_psum(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i/VALUE_GROUP;
	uint64_t cpsum = psumx.prefixsum(p);
	if (r == 0) return cpsum;
	size_t pos = ptr.prefixsum(p);
	mscds::IWBitStream is(enc.data_ptr(), enc.length(), pos);
	coder::DeltaCoder dc;
	coder::CodePr c;
	for (unsigned int j = 0; j < r; ++j) {
		c = dc.decode2(is.peek());
		is.skipw(c.second);
		cpsum += range_len(p*VALUE_GROUP + j) * c.first;
	}
	return cpsum; 
}

RunLenSumArray3::RunLenSumArray3(): len(0) {}

unsigned int RunLenSumArray3::count_range(unsigned int pos) const {
	return 0;
}

uint64_t RunLenSumArray3::sum(uint32_t pos) const {
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
		val = range_value(p+1);
		return  pm + kl*(val);
	} else
		return range_psum(p+1);
}

int64_t RunLenSumArray3::sum_delta(uint32_t pos, int64_t delta) const {
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
		val = range_value(p+1);
		return  pm + kl*(val) + delta * (kl + ps);
	} else
		return range_psum(p+1) + delta*(rangelen + ps);
}

unsigned int RunLenSumArray3::countnz(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint64_t ps = 0;
	uint32_t rangelen = rlen.lookup(p, ps);
	return std::min<uint32_t>((pos - sp), rangelen) + ps;
}

unsigned int RunLenSumArray3::access(unsigned int pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint32_t rangelen = rlen.lookup(p);
	if (rangelen > (pos - sp)) {
		return range_value(p+1);
	} else
		return 0;
}

int RunLenSumArray3::prev(unsigned int pos) const {
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

int RunLenSumArray3::next(unsigned int pos) const {
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

unsigned int RunLenSumArray3::length() const {
	return len;
}

void RunLenSumArray3::clear() {
	len = 0;
	start.clear();
	rlen.clear();
	ptr.clear();
	enc.clear();
}

}//namespace