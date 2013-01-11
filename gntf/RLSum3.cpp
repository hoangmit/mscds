#include "RLSum3.h"



#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace app_ds {
using namespace mscds;

uint64_t wrap(int64_t v) {
	if (v == 0) return 0;
	else if (v > 0) return v*2 - 1;
	else return (-v)*2;
}

void RunLenSumArrayBuilder3::clear() {
	len = 0;
	lastst = 0;
	stbd.clear();
	rlbd.clear();
	pspt.clear();
	psum = 0;
	lastv = 0;
	cnt++;
}

void RunLenSumArrayBuilder3::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (llen == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	//psbd.add(llen * v);
	stbd.add_inc(st);
	rlbd.add(llen);
	lastst = st;
	if (cnt % 256 == 0) {
		pspt.add_inc(enc.length());
	}else {
		//uint64_t vx = ((int64_t)v) - lastv;
		//coder::CodePr c = dc.encode(wrap(vx)+1);
		//enc.puts(c.first, c.second);
	}
		coder::CodePr c = dc.encode(v);
		enc.puts(c.first, c.second);
		lastv = v;
	cnt++;
}

void RunLenSumArrayBuilder3::build(RunLenSumArray3 *out) {
	out->clear();
	enc.close();
	out->len = len;
	stbd.build(&(out->start));
	rlbd.build(&(out->rlen));
	pspt.build(&(out->ptr));
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

unsigned int RunLenSumArray3::range_start(unsigned int i) const { return 0; }
uint64_t RunLenSumArray3::range_psum(unsigned int i) const { return 0; }
unsigned int RunLenSumArray3::range_len(unsigned int i) const { return 0; }
unsigned int RunLenSumArray3::range_value(unsigned int i) const { return 0; }
unsigned int RunLenSumArray3::pslen(unsigned int i) const { return 0; }

RunLenSumArray3::RunLenSumArray3(): len(0) {}

unsigned int RunLenSumArray3::count_range(unsigned int pos) const {
	return 0;
}

uint64_t RunLenSumArray3::sum(uint32_t pos) const {
	return 0;
}

int64_t RunLenSumArray3::sum_delta(uint32_t pos, int64_t delta) const {
	return 0;
}

unsigned int RunLenSumArray3::countnz(unsigned int pos) const {
	return 0;
}

unsigned int RunLenSumArray3::access(unsigned int pos) const {
	return 0;
}

int RunLenSumArray3::prev(unsigned int pos) const {
	return 0;
}

int RunLenSumArray3::next(unsigned int pos) const {
	return 0;
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