#include "SampledSum.h"
#include <cmath>

namespace app_ds {

static double floatval(double r) {return (r > 0.0) ? r - floor(r) : ceil(r) - r; }

double roundn(double r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

unsigned int SampledSumBuilder::precision(double d) {
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

void SampledSumBuilder::init(unsigned int sample_rate) {
	clear();
	this->sample_rate = sample_rate;
}

void SampledSumBuilder::clear() {
	psbd.clear();
	spsbd.clear();
	lastst = 0;
	psum = 0;
	lastv = 0;
	svals.clear();
	ptr = &svals;
	factor = 1;
	delta = 0;
	cnt = 0;
}

void SampledSumBuilder::add(unsigned int st, unsigned int ed, double val) {
	if (ed - st == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");

	svals.push_back(ValRange(st, ed, val));
	lastst = st;
}

void SampledSumBuilder::build(mscds::OArchive& ar) {
	SampledSumQuery a;
	build(&a);
	a.save(ar);
}

void SampledSumBuilder::add_all( std::deque<ValRange>* vals ) {
	ptr = vals;
}


void SampledSumBuilder::build(SampledSumQuery * out) {
	out->clear();
	comp_transform();
	lastst = 0;
	psum = 0;
	lastv = 0;
	cnt = 0;
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		addint(it->st, it->ed, it->val * factor + delta);
	if (cnt % sample_rate == 0) {
		psbd.add_inc(psum);
		spsbd.add_inc(sqpsum);
	}
	out->len = ptr->size();
	
	psbd.build(&(out->psum));
	spsbd.build(&(out->sqrsum));
	vals.build(&(out->vals));
	out->factor = factor;
	out->delta = delta;
	out->rate = sample_rate;
	clear();

}

void SampledSumBuilder::comp_transform() {
	unsigned int pc = 0;
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		pc = std::max<unsigned int>(precision(it->val), pc);
	factor = 1;
	if (factor > 5) factor = 5;
	for (unsigned int i = 0; i < pc; ++i) factor *= 10;
	int minr = std::numeric_limits<int>::max();
	for (auto it = ptr->begin(); it != ptr->end(); ++it)
		minr = std::min<int>(minr, it->val*factor);
	delta = 1 - minr;
}

void SampledSumBuilder::addint(unsigned int st, unsigned int ed, unsigned int v) {
	unsigned int llen = ed - st;
	if (st < lastst) throw std::runtime_error("required sorted array");
	//psbd.add(llen * v);
	
	lastst = st;
	if (cnt % sample_rate == 0) {
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




double SampledSumQuery::getValue(unsigned int idx) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	PRValArr::Enumerator g;
	vals.getEnum(p*rate, &g);
	double x;
	for (size_t i = 0; i < r; ++i) g.next();
	x = g.next();
	return (x - delta) / (double) factor;
}


unsigned int SampledSumQuery::getEnum(unsigned int idx, Enum * e) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	vals.getEnum(p*rate, e);
	return p * rate;
}


double SampledSumQuery::sum(unsigned int idx, unsigned int lefpos) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	int64_t cpsum = pq->int_psrlen(idx - r) * delta;
	cpsum = psum.prefixsum(p+1) - cpsum;
	PRValArr::Enumerator g;
	if (r > 0 || lefpos > 0) {
		vals.getEnum(p*rate, &g);
		//PNIntv::Enum rle;
		//itv.getLenEnum(p*rate, &rle);
		for (size_t j = 0; j < r; ++j) {
			// assert(rle.next() == range_len(p*rate + j));
			cpsum += (g.next() - delta) * pq->int_len(p*rate + j);
		}
	} 
	if (lefpos > 0) cpsum += (g.next() - delta) * lefpos;
	return cpsum/(double)factor;
}




void SampledSumQuery::save(mscds::OArchive& ar) const {

}

void SampledSumQuery::load(mscds::IArchive& ar, NIntvQueryInt * posquery) {
	this->pq = posquery;

}

void SampledSumQuery::clear()
{

}




}