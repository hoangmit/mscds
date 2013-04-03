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

static double floatval(double r) {return (r > 0.0) ? r - floor(r) : ceil(r) - r; }

double roundn(double r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

unsigned int RunLenSumArrayBuilder6::precision(double d) {
	return -floor(std::log10(fabs(d - roundn(d))));

	unsigned int p = 0;
	d = fabs(d);
	d = d - floor(d);
	while (d > 1e-5 && p < 6) {
		d*=10;
		d -= floor(d);
		p++;
	}
	return p;
}


void RunLenSumArrayBuilder6::clear() {
	lastst = 0;
	itvb.clear();
	psbd.clear();
	spsbd.clear();
	psum = 0;
	lastv = 0;
	cnt = 0;
	svals.clear();
	ptr = &svals;
	factor = 1;
	delta = 0;
}

const unsigned int VALUE_GROUP = 64;

void RunLenSumArrayBuilder6::add(unsigned int st, unsigned int ed, double v) {
	if (ed - st == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");
	
	svals.push_back(ValRange(st, ed, v));
	lastst = st;
}

void RunLenSumArrayBuilder6::build(RunLenSumArray6 *out) {
	out->clear();
	comp_transform();
	lastst = 0;
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		addint(it->st, it->ed, it->val * factor + delta);
	out->len = ptr->size();
	itvb.build(&(out->itv));
	psbd.build(&(out->psum));
	spsbd.build(&(out->sqrsum));
	vals.build(&(out->vals));
	out->factor = factor;
	out->delta = delta;
	clear();
}

void RunLenSumArrayBuilder6::build(OArchive& ar) {
	RunLenSumArray6 a;
	build(&a);
	a.save(ar);
}

void RunLenSumArrayBuilder6::comp_transform() {
	unsigned int pc = 0;
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		pc = std::max<unsigned int>(precision(it->val), pc);
	factor = 1;
	if (factor > 4) factor = 4;
	for (unsigned int i = 0; i < pc; ++i) factor *= 10;
	int minr = std::numeric_limits<int>::max();
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		minr = std::min<int>(minr, it->val*factor);
	delta = 1 - minr;
}

void RunLenSumArrayBuilder6::addint(unsigned int st, unsigned int ed, unsigned int v) {
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

//-----------------------------------------------------------------------------

void RunLenSumArray6::save(OArchive& ar) const {
	ar.startclass("run_length_sum_array3", 1);
	ar.var("len").save(len);
	ar.var("factor").save(factor);
	ar.var("delta").save(delta);
	itv.save(ar.var("intervals"));
	vals.save(ar.var("vals"));
	psum.save(ar.var("sampled_psum"));
	sqrsum.save(ar.var("sampled_sqrsum"));
	ar.endclass();
}

void RunLenSumArray6::load(IArchive& ar) {
	clear();
	ar.loadclass("run_length_sum_array3");
	ar.var("len").load(len);
	ar.var("factor").load(factor);
	ar.var("delta").load(delta);
	itv.load(ar.var("intervals"));
	vals.load(ar.var("vals"));
	psum.load(ar.var("sampled_psum"));
	sqrsum.load(ar.var("sampled_sqrsum"));
	ar.endclass();
}

unsigned int RunLenSumArray6::range_start(unsigned int i) const { return itv.int_start(i); }
unsigned int RunLenSumArray6::range_len(unsigned int i) const { return itv.int_len(i); }

double RunLenSumArray6::range_value(unsigned int i) const {
	size_t r = i % VALUE_GROUP;
	size_t p = i / VALUE_GROUP;
	PRSumArr::Enumerator g;
	vals.getEnum(p*VALUE_GROUP, &g);
	double x;
	for (size_t i = 0; i < r; ++i) g.next();
	x = g.next();
	return (x - delta) / (double) factor;
}

unsigned int RunLenSumArray6::count_range(unsigned int pos) const {
	auto res = itv.find_cover(pos);
	if (res.first == 0 && res.second == 0) return 0;
	else return res.first + 1;
}

double RunLenSumArray6::range_psum(unsigned int i) const {
	return sum(range_start(i));
}

double RunLenSumArray6::sum(uint32_t pos) const {
	if (pos == 0) return 0;
	auto res = itv.find_cover(pos - 1);
	if (res.first == 0 && res.second == 0) return 0;
	size_t r = res.first % VALUE_GROUP;
	size_t p = res.first / VALUE_GROUP;
	int64_t cpsum = psum.prefixsum(p+1);
	cpsum -= itv.int_psrlen(res.first - r) * delta;
	PRSumArr::Enumerator g;
	if (r > 0 || res.second > 0) {
		vals.getEnum(p*VALUE_GROUP, &g);
		for (size_t j = 0; j < r; ++j)
			cpsum += (g.next() - delta) * range_len(p*VALUE_GROUP + j);
	} 
	if (res.second > 0) cpsum += (g.next() - delta) * res.second;
	return cpsum/(double)factor;
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

unsigned int RunLenSumArray6::length() const {
	return len;
}

void RunLenSumArray6::clear() {
	len = 0;
	itv.clear();
	vals.clear();
}

}//namespace
