#include "chrfmt.h"
#include "mem/filearchive.h"

#include <stdexcept>
using namespace std;
using namespace mscds;

namespace app_ds {

ChrNumThreadBuilder::ChrNumThreadBuilder():setup_(false){}

void ChrNumThreadBuilder::init(minmaxop_t option, unsigned int factor, bool range_annotations) {
	this->factor = factor;
	this->minmax_opt = option;
	this->has_annotation = range_annotations;
	setup_ = true;
}

void ChrNumThreadBuilder::add(unsigned int st, unsigned int ed, int val, const std::string& s) {
	if (!setup_) throw std::runtime_error("need setup");
	//if (val == 0) std::runtime_error("zero value with annotation");
	if (val != 0)
		ranges.push_back(ValRange(st, ed, val));
	if (has_annotation)
		annbd.add(s);
}

void ChrNumThreadBuilder::build(mscds::OArchive& ar) {
	ChrNumThread t;
	build(&t);
	t.save(ar);
}

int ChrNumThreadBuilder::find_min() {
	int minv = std::numeric_limits<int>::max();
	for (auto it = ranges.begin(); it != ranges.end(); ++it)
		if (minv > it->val)
			minv = it->val;
	return minv;
}

void ChrNumThreadBuilder::build(ChrNumThread* out) {
	out->clear();
	if(!is_sorted(ranges.begin(), ranges.end()))
		std::sort(ranges.begin(), ranges.end());
	out->factor = factor;
	int64_t delta = 1 - find_min();	
	out->delta = -delta;
	out->minmax_opt = minmax_opt;
	std::vector<int> minmaxr(ranges.size());
	RunLenSumArrayBuilder bd;
	for (auto it = ranges.begin(); it != ranges.end(); ++it) {
		bd.add(it->st, it->ed, (unsigned int) (it->val + delta));
		minmaxr.push_back(it->val);
	}
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
	ar.var("factor").load(factor);
	ar.var("delta").load(delta);
	uint32_t o = minmax_opt;
	ar.var("minmax_opt").load(o);
	minmax_opt = (minmaxop_t) o;
	vals.load(ar.var("values"));
	min.load(ar.var("min"));
	min.load(ar.var("max"));
	ar.var("annotation_opt").load(o);
	if (o != 0)
		annotations.load(ar.var("annotation"));
	ar.endclass();
}

void ChrNumThread::save(mscds::OArchive& ar) const {
	ar.startclass("chromosome_number_thread", 1);
	save_str(ar.var("chr_name"), name);
	ar.var("factor").save(factor);
	ar.var("delta").save(delta);
	uint32_t o = minmax_opt;
	ar.var("minmax_opt").save(o);
	vals.save(ar.var("values"));
	min.save(ar.var("min"));
	min.save(ar.var("max"));
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
			<< ed << " " << (val + delta);
		if (has_annotation)
			fo << " " << annotations.get(i);
		fo << '\n';
	}
}

std::vector<long long> ChrNumThread::sum_batch(size_t st, size_t ed, size_t n) const {
	std::vector<long long> ret(n);
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

std::vector<unsigned int> ChrNumThread::count_range_batch(size_t st, size_t ed, size_t n) const {
	std::vector<unsigned int> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t pos = st;
	uint64_t lval = count_range(st);
	for (size_t i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = count_range(pos) - lval;
	}
	for (size_t i = r; i < n; ++i) {
		pos += d;
		ret[i] = count_range(pos) - lval;
	}
	return ret;
}

std::vector<unsigned int> ChrNumThread::count_nz_batch(unsigned int st, size_t ed, size_t n) const {
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


std::vector<unsigned int> ChrNumThread::min_value_batch(unsigned int st, size_t ed, size_t n) const {
	std::vector<unsigned int> ret(n);
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

std::vector<unsigned int> ChrNumThread::max_value_batch(unsigned int st, size_t ed, size_t n) const {
	std::vector<unsigned int> ret(n);
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
