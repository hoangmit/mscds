#include "chrfmt.h"
#include "mem/filearchive.h"

#include <stdexcept>
#include <cstring>
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
	ChrNumValType::Enum e;
	vals.getEnum(0, &e);
	unsigned int i = 0;
	unsigned int BUFSIZE = 4096;
	char buffer[BUFSIZE];
	if (!has_annotation) {
		while (e.hasNext()) {
			auto x = e.next();
			unsigned len = snprintf(buffer, BUFSIZE, "%s\t%d\t%d\t%f", name.c_str(), x.st, x.ed, x.val);
			fo.write(buffer, len + 1);
			//fo << name << '\t' << x.st << '\t' << x.ed << '\t' << x.val << '\n';
			++i;
		}
	}else {
		while (e.hasNext()) {
			auto x = e.next();
			fo << name << '\t' << x.st << '\t' << x.ed << '\t' << x.val;
			fo << '\t' <<  vals.range_value(i) << '\n';
			++i;
		}
	}
	assert(i == vals.length());

	/*
	for (unsigned int i = 0; i < vals.length(); ++i) {
		unsigned int st = vals.range_start(i);
		unsigned int ed = st + vals.range_len(i);
		fo << name << '\t' << st << '\t'
			<< ed << '\t' << vals.range_value(i);
		if (has_annotation)
			fo << " " << annotations.get(i);
		fo << '\n';
	}*/

}

unsigned int ChrNumThread::count_intervals() const {
	return vals.length();
}

double ChrNumThread::stdev(unsigned int st, unsigned int ed) const {
	return 0;
}

std::vector<double> ChrNumThread::stdev_batch( unsigned int st, unsigned int ed, unsigned int n ) const{
	return std::vector<double>();
}

template<typename Tp, typename Func>
std::vector<Tp> batch_call1(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	std::vector<Tp> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t pos = st;
	Tp lval = fx(st);
	for (unsigned int i = 0; i < r; ++i) {
		pos += d + 1;
		Tp rval = fx(pos);
		ret[i] = rval - lval;
		lval = rval;
	}
	for (unsigned int i = r; i < n; ++i) {
		pos += d;
		Tp rval = fx(pos);
		ret[i] = rval - lval;
		lval = rval;
	}
	return ret;
}

template<typename Tp, typename Func>
std::vector<Tp> batch_call2(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	std::vector<Tp> ret(n);
	assert(ed - st >= n);
	size_t d = (ed - st) / n;
	size_t r = (ed - st) % n;
	size_t lpos = st, pos = st;
	for (unsigned int i = 0; i < r; ++i) {
		pos += d + 1;
		ret[i] = fx(lpos, pos);
		lpos = pos;
	}
	for (unsigned int i = r; i < n; ++i) {
		pos += d;
		ret[i] = fx(lpos, pos);
		lpos = pos;
	}
	return ret;
}

std::vector<double> ChrNumThread::sum_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	return batch_call1<double>(st, ed, n, [&](size_t pos)->double {return this->sum(pos);});
}

std::vector<unsigned int> ChrNumThread::count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	return batch_call1<unsigned int>(st, ed, n, [&](size_t pos)->unsigned int {return this->count_intervals(pos);});
}

std::vector<unsigned int> ChrNumThread::coverage_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	return batch_call1<unsigned int>(st, ed, n, [&](size_t pos)->unsigned int {return this->coverage(pos);});
}

std::vector<double> ChrNumThread::min_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	return batch_call2<double>(st, ed, n, [&](size_t st, size_t ed)->double{ return this->min_value(st, ed); });
}

std::vector<double> ChrNumThread::max_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	return batch_call2<double>(st, ed, n, [&](size_t st, size_t ed)->double{ return this->max_value(st, ed); });
}

std::vector<double> ChrNumThread::avg_batch( unsigned int st, unsigned int ed, unsigned int n ) const {
	std::vector<double> s(sum_batch(st, ed, n));
	std::vector<unsigned int> l(coverage_batch(st, ed, n));
	unsigned int lr = (ed - st) / n;
	for (unsigned int i = 0; i < s.size(); ++i)
		s[i] = s[i] / l[i];
	return s;
}






}//namespace
