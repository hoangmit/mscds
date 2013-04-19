#include "chrfmt.h"
#include "mem/filearchive.h"

#include <stdexcept>
using namespace std;
using namespace mscds;

namespace app_ds {

ChrNumThreadBuilder::ChrNumThreadBuilder():setup_(false){}

void ChrNumThreadBuilder::init(minmaxop_t option, bool range_annotations) {
	this->minmax_opt = option;
	this->has_annotation = range_annotations;
	setup_ = true;
}

void ChrNumThreadBuilder::add(unsigned int st, unsigned int ed, double val, const std::string& s) {
	if (!setup_) throw std::runtime_error("need setup");
	//if (val == 0) std::runtime_error("zero value with annotation");
	ranges.push_back(ValRange(st, ed, val));
	if (has_annotation)
		annbd.add(s);
}

void ChrNumThreadBuilder::build(mscds::OArchive& ar) {
	ChrNumThread t;
	build(&t);
	t.save(ar);
}

void ChrNumThreadBuilder::build(ChrNumThread* out) {
	out->clear();
	if(!is_sorted(ranges.begin(), ranges.end()))
		std::sort(ranges.begin(), ranges.end());
	out->minmax_opt = minmax_opt;
	std::vector<double> minmaxr(ranges.size());
	ChrNumValBuilderType bd;
	unsigned int i = 0;
	for (auto it = ranges.begin(); it != ranges.end(); ++it) {
		minmaxr[i] = it->val;
		++i;
	}
	bd.add_all(&ranges);
	bd.build(&out->vals);
	ranges.clear();

	if (out->minmax_opt & MIN_OP) {
		BitArray b = build_supercartisian_tree(true, minmaxr.begin(), minmaxr.end());
		out->min.build(b, 512);
	}
	if (out->minmax_opt & MAX_OP) {
		BitArray b = build_supercartisian_tree(false, minmaxr.begin(), minmaxr.end());
		out->max.build(b, 512);
	}
	out->has_annotation = has_annotation;
	if (has_annotation) {
		annbd.build(&out->annotations);
	}
	setup_ = false;
}

//-----------------------------------------------------------------------------

void ChrNumThread::clear() {
	vals.clear();
	min.clear();
	max.clear();
	name.clear();
}

void ChrNumThread::load(mscds::IArchive& ar) {
	clear();
	ar.loadclass("chromosome_number_thread");
	name = load_str(ar.var("chr_name"));
	uint32_t o = minmax_opt;
	ar.var("minmax_opt").load(o);
	minmax_opt = (minmaxop_t) o;
	vals.load(ar.var("values"));
	min.load(ar.var("min"));
	max.load(ar.var("max"));
	ar.var("annotation_opt").load(o);
	if (o != 0) {
		has_annotation = true;
		annotations.load(ar.var("annotation"));
	}else has_annotation = false;
	ar.endclass();
}

void ChrNumThread::save(mscds::OArchive& ar) const {
	ar.startclass("chromosome_number_thread", 1);
	save_str(ar.var("chr_name"), name);
	uint32_t o = minmax_opt;
	ar.var("minmax_opt").save(o);
	vals.save(ar.var("values"));
	min.save(ar.var("min"));
	max.save(ar.var("max"));
	o = has_annotation;
	ar.var("annotation_opt").save(o);
	if (has_annotation)
		annotations.save(ar.var("annotation"));
	ar.endclass();
}

void ChrNumThread::dump_bedgraph(std::ostream& fo) const {
	for (size_t i = 0; i < vals.length(); ++i) {
		unsigned int st = vals.range_start(i);
		unsigned int ed = st + vals.range_len(i);
		int val = vals.range_value(i);
		fo << name << " " << st
			<< ed << " " << (val);
		if (has_annotation)
			fo << " " << annotations.get(i);
		fo << '\n';
	}
}

std::vector<double> ChrNumThread::sum_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<double> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t pos = st;
	uint64_t lval = sum(st);
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = sum(pos) - lval;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = sum(pos) - lval;
	}
	return ret;
}

std::vector<unsigned int> ChrNumThread::count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<unsigned int> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t pos = st;
	uint64_t lval = count_intervals(st);
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = count_intervals(pos) - lval;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = count_intervals(pos) - lval;
	}
	return ret;
}

std::vector<unsigned int> ChrNumThread::count_nz_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<unsigned int> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t pos = st;
	uint64_t lval = count_nz(st);
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = count_nz(pos) - lval;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = count_nz(pos) - lval;
	}
	return ret;
}


std::vector<double> ChrNumThread::min_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<double> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t lpos = st, pos = st;
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = min_value(lpos, pos);
		lpos = pos;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = min_value(lpos, pos);
		lpos = pos;
	}
	return ret;
}

std::vector<double> ChrNumThread::max_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<double> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t lpos = st, pos = st;
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = max_value(lpos, pos);
		lpos = pos;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = max_value(lpos, pos);
		lpos = pos;
	}
	return ret;
}

}//namespace
