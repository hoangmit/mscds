#include "RLSum2.h"



#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

namespace app_ds {
using namespace mscds;

void RunLenSumArrayBuilder2::clear() {
	len = 0;
	lastst = 0;
	psbd.clear();
	posbd.clear();
}

void RunLenSumArrayBuilder2::add(unsigned int st, unsigned int ed, unsigned int v) {
	len++;
	unsigned int llen = ed - st;
	if (llen == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	psbd.add(llen * v);
	posbd.add_inc(st);
	posbd.add_inc(ed-1);
	lastst = st;
}

void RunLenSumArrayBuilder2::build(RunLenSumArray2 *out) {
	out->clear();
	out->len = len;
	psbd.build(&(out->psum));
	posbd.build(&(out->pos));
	clear();
}

void RunLenSumArrayBuilder2::build(OArchive& ar) {
	ar.startclass("run_length_sum_array2", 1);
	ar.var("len").save(len);
	posbd.build(ar.var("positions"));
	psbd.build(ar.var("prefixsum"));
	ar.endclass();
	clear();
}

//-----------------------------------------------------------------------------

void RunLenSumArray2::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array2", 1);
	ar.var("len").save(len);
	pos.save(ar.var("positions"));
	psum.save(ar.var("prefixsum"));
	ar.endclass();
}

void RunLenSumArray2::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array2");
	ar.var("len").load(len);
	pos.load(ar.var("positions"));
	psum.load(ar.var("prefixsum"));
	ar.endclass();
}

unsigned int RunLenSumArray2::range_start(unsigned int i) const { return 0; }
uint64_t RunLenSumArray2::range_psum(unsigned int i) const { return 0; }
unsigned int RunLenSumArray2::range_len(unsigned int i) const { return 0; }
unsigned int RunLenSumArray2::range_value(unsigned int i) const { return 0; }
unsigned int RunLenSumArray2::pslen(unsigned int i) const { return 0; }

RunLenSumArray2::RunLenSumArray2(): len(0) {}

unsigned int RunLenSumArray2::count_range(unsigned int pos) const {
	return 0;
}

uint64_t RunLenSumArray2::sum(uint32_t pos) const {
	return 0;
}

int64_t RunLenSumArray2::sum_delta(uint32_t pos, int64_t delta) const {
	return 0;
}

unsigned int RunLenSumArray2::countnz(unsigned int pos) const {
	return 0;
}

unsigned int RunLenSumArray2::access(unsigned int pos) const {
	return 0;
}

int RunLenSumArray2::prev(unsigned int pos) const {
	return 0;
}

int RunLenSumArray2::next(unsigned int pos) const {
	return 0;
}

unsigned int RunLenSumArray2::length() const {
	return len;
}

void RunLenSumArray2::clear() {
	len = 0;
	pos.clear();
	psum.clear();
}

}//namespace