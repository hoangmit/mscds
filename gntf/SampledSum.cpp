#include "SampledSum.h"
#include <cmath>
#include "utils/param.h"

namespace app_ds {

static double floatval(double r) {return (r > 0.0) ? r - floor(r) : ceil(r) - r; }

static double roundn(double r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

unsigned int SampledSumBuilder::precision(double d) {
	double v = d - roundn(d);
	return 0 ? v == 0 : -floor(std::log10(fabs(v)));

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
	method = 0;
}

void SampledSumBuilder::add(unsigned int st, unsigned int ed, double val) {
	if (ed - st == 0) throw std::runtime_error("zero length range");
	if (st < lastst) throw std::runtime_error("required sorted array");

	svals.push_back(ValRange(st, ed, val));
	lastst = st;
}

void SampledSumBuilder::build(mscds::OArchive& ar) {
	SampledSumQuery a;
	build(&a, NULL);
	a.save(ar);
}

void SampledSumBuilder::add_all( std::deque<ValRange>* vals ) {
	ptr = vals;
}


void SampledSumBuilder::build(SampledSumQuery * out, NIntvQueryInt * posquery) {
	out->clear();
	out->pq = posquery;
	auto cf = Config::getInst();
	unsigned int storemed = cf->getIntPara("GNTF.VALUE_STORAGE", 0);
	if (storemed > 3) throw std::runtime_error("invalid method");

	if (method == 0) {
		method = cf->getIntPara("GNTF.INT_STORAGE", 0);
		if (method > 2) throw std::runtime_error("invalid method");
	}
	comp_transform();
	assert(method == 1 || method == 2);
	if (method == 1) vdir.init(storemed, sample_rate);
	else vrank.init(storemed, sample_rate);
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
	if (method == 1) vdir.build(&(out->vdir));
	else vrank.build(&(out->vrank));
	out->factor = factor;
	out->delta = delta;
	out->rate = sample_rate;
	out->method = method;
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
	if (method == 0) {
		if (factor == 1) method = 1;
		else method = 2;
	}
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
	if (method == 1) vdir.add(v);
	else vrank.add(v);
	lastv = v;

	psum += llen * v;
	sqpsum += llen * (v*v);
	cnt++;
}


double SampledSumQuery::access(unsigned int idx) const {
	double x = 0;
	if (method == 1) x = vdir.access(idx);
	else x = vrank.access(idx);
	return (x - delta) / (double) factor;
}

void SampledSumQuery::getEnum(unsigned int idx, Enum * e) const {
	e->type = method;
	if (method == 1) {
		if (e->e1 == NULL) e->e1 = new PRValArr::Enum();
		vdir.getEnum(idx, e->e1);
	} else if (method == 2) {
		if (e->e2 == NULL) e->e2 = new RankValArr::Enum();
		vrank.getEnum(idx, e->e2);
	}
}

double SampledSumQuery::sum(unsigned int idx, unsigned int lefpos) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	int64_t cpsum = pq->int_psrlen(idx - r) * delta;
	cpsum = psum.prefixsum(p+1) - cpsum;
	Enum g;
	if (r > 0 || lefpos > 0) {
		getEnum(p*rate, &g);
		//PNIntv::Enum rle;
		//itv.getLenEnum(p*rate, &rle);
		for (size_t j = 0; j < r; ++j) {
			// assert(rle.next() == range_len(p*rate + j));
			cpsum += (g.next_int() - delta) * pq->int_len(p*rate + j);
		}
	} 
	if (lefpos > 0) cpsum += (g.next_int() - delta) * lefpos;
	return cpsum/(double)factor;
}

void SampledSumQuery::save(mscds::OArchive& ar) const {
	ar.startclass("sampledsumquery");
	ar.var("int_method_type").save(method);
	ar.var("rate").save(rate);
	ar.var("delta").save(delta);
	ar.var("factor").save(factor);
	psum.save(ar.var("psum"));
	sqrsum.save(ar.var("sqrsum"));
	if (method == 1) {
		vdir.save(ar.var("direct_values"));
	}else 
		if (method == 2) {
			vrank.save(ar.var("rank_values"));
		}else throw mscds::ioerror("wrong method range");
	ar.endclass();
}

void SampledSumQuery::load(mscds::IArchive& ar, NIntvQueryInt * posquery) {
	this->pq = posquery;
	ar.loadclass("sampledsumquery");
	ar.var("int_method_type").load(method);
	ar.var("rate").load(rate);
	ar.var("delta").load(delta);
	ar.var("factor").load(factor);
	psum.load(ar.var("psum"));
	sqrsum.load(ar.var("sqrsum"));
	if (method == 1) {
		vdir.load(ar.var("direct_values"));
	}else 
		if (method == 2) {
			vrank.load(ar.var("rank_values"));
		}else throw mscds::ioerror("wrong method range");
	ar.endclass();	
}

void SampledSumQuery::clear() {
	pq = NULL;
	psum.clear();
	sqrsum.clear();
	len = 0;
	rate = 0;
	factor = 0;
	delta = 0;
	method = 0;
}

SampledSumQuery::SampledSumQuery() {
	clear();
}



bool SampledSumQuery::Enum::hasNext() const {
	if (type == 1) return e1->hasNext();
	else if (type == 2) return e2->hasNext();
	else return false;
}

SampledSumQuery::Enum::~Enum() {
	if (e1 != NULL) delete e1;
	if (e2 != NULL) delete e2;
}

double SampledSumQuery::Enum::next() {
	return (double)(next_int() - delta) / factor;
}

uint64_t SampledSumQuery::Enum::next_int() {
	if (type == 1) return e1->next();
	else if (type == 2) return e2->next();
	else return 0;
}




}