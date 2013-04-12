#include "RLSum6.h"
#include "valrange.h"

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <limits>

using namespace std;

namespace app_ds {
using namespace mscds;


void RunLenSumArrayBuilder6::clear() {
	itvb.clear();
	vals.clear();
	cnt = 0;
}

void RunLenSumArrayBuilder6::add(unsigned int st, unsigned int ed, double v) {
	cnt++;
	itvb.add(st, ed);
	vals.add(st, ed, v);
}

void RunLenSumArrayBuilder6::add_all(std::deque<ValRange> * vs) {
	vals.add_all(vs);
	for (auto it = vs->begin(); it != vs->end(); ++it) 
		itvb.add(it->st, it->ed);
}

void RunLenSumArrayBuilder6::build(RunLenSumArray6 *out) {
	out->clear();
	out->len = cnt;
	itvb.build(&(out->itv));
	vals.build(&(out->vals));
	clear();
}

void RunLenSumArrayBuilder6::build(OArchive& ar) {
	RunLenSumArray6 a;
	build(&a);
	a.save(ar);
}

//-----------------------------------------------------------------------------

void RunLenSumArray6::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array3", 1);
	ar.var("len").save(len);
	itv.save(ar.var("intervals"));
	vals.save(ar.var("vals"));
	ar.endclass();
}

void RunLenSumArray6::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array3");
	ar.var("len").load(len);
	itv.load(ar.var("intervals"));
	vals.load(ar.var("vals"), &itv);
	ar.endclass();
}

unsigned int RunLenSumArray6::range_start(unsigned int i) const { return itv.int_start(i); }
unsigned int RunLenSumArray6::range_len(unsigned int i) const { return itv.int_len(i); }

double RunLenSumArray6::range_value(unsigned int i) const {
	return vals.getValue(i);
}

double RunLenSumArray6::range_psum(unsigned int i) const {
	return sum(range_start(i));
}

double RunLenSumArray6::sum(uint32_t pos) const {
	if (pos == 0) return 0;
	auto res = itv.find_cover(pos - 1);
	if (res.first == 0 && res.second == 0) return 0;

	return vals.sum(res.first);
}

unsigned int RunLenSumArray6::countnz(unsigned int pos) const {
	return itv.coverage(pos);
}

double RunLenSumArray6::access(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second != 0) return range_value(res.first);
	else return 0;
}

int RunLenSumArray6::prev(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second > 0) return pos;
	else if (res.first > 0) return itv.int_end(res.first - 1) - 1;
	else return -1;
}

int RunLenSumArray6::next(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.second > 0) return pos;
	else if (res.first < len) return itv.int_start(res.first);
	else return -1;
}

unsigned int RunLenSumArray6::count_range(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.first == 0 && res.second == 0) return 0;
	else return res.first + 1;
}

std::pair<int, int> RunLenSumArray6::find_intervals(unsigned int st, unsigned int ed) const {
	std::pair<unsigned int, unsigned int> ret;
	auto r1 = itv.find_cover(st);
	auto r2 = itv.find_cover(ed);
	if (r1.second > 0) ret.first = r1.first;
	else ret.first = r1.first + 1;
	if (r2.second > 0) ret.second = r2.first;
	else r2.first = r2.first - 1;
	return ret;
}


unsigned int RunLenSumArray6::length() const {
	return len;
}

void RunLenSumArray6::clear() {
	len = 0;
	itv.clear();
	vals.clear();
}

}//namespace
