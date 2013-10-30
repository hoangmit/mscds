#include "chrfmt.h"
#include "mem/filearchive.h"
#include "utils/modp_numtoa.h"

#include <cstdio>
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

void ChrNumThread::getEnum(unsigned int i, ChrNumValType::Enum* e) const {
	vals.getEnum(i, e);
}

void ChrNumThread::dump_bedgraph(std::ostream& fo) const {
	ChrNumValType::Enum e;
	vals.getEnum(0, &e);
	unsigned int i = 0;
	const unsigned int BUFSIZE = 4096;
	char buffer[BUFSIZE];
	if (name.length() > BUFSIZE / 2) throw std::runtime_error("chromosome name too long");
	if (!has_annotation) {
		while (e.hasNext()) {
			auto x = e.next();
			unsigned len = sprintf(buffer, "%s\t%d\t%d\t", name.c_str(), x.st, x.ed);
			unsigned l2 = utils::modp_dtoa2(x.val, buffer + len, 6);
			buffer[len + l2] = '\n';
			fo.write(buffer, len + l2 + 1);
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
	if (st >= ed) throw runtime_error("invalid input interval");
	return 0;
}

std::vector<double> ChrNumThread::stdev_batch( unsigned int st, unsigned int ed, unsigned int n ) const{
	return std::vector<double>();
}

template<typename Tp, typename Func>
std::vector<Tp> batch_call1(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	assert(ed - st >= n);
	if (st >= ed) throw runtime_error("invalid input interval");
	//assert(n > 0);
	if (n == 0) throw runtime_error("zero length window size");
	std::vector<Tp> ret(n);
	unsigned int l = (ed - st), dt = l / n, r = l % n;
	int A = n - r, B = r;
	int sl = 0;
	unsigned int pos = st;
	Tp lval = fx(pos);
	for (int i = 0; i < n; ++i) {
		if (sl + A <= B) {
			sl += 2 * A;
			pos += dt + 1;
		} else {
			sl -= 2 * B;
			pos += dt;
		}
		Tp cval = fx(pos);
		ret[i] = cval - lval;
		lval = cval;
	}
	assert(ed == pos);
	return ret;
}

template<typename Tp, typename Func>
std::vector<Tp> batch_call2(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	assert(ed - st >= n);
	assert(st < ed);
	//assert(n > 0);
	if (n == 0) throw runtime_error("zero length input interval");
	std::vector<Tp> ret(n);
	unsigned int l = (ed - st), dt = l / n, r = l % n;
	int A = n - r, B = r;
	int sl = 0;
	unsigned int lpos = st, pos = st;
	for (int i = 0; i < n; ++i) {
		if (sl + A <= B) {
			sl += 2 * A;
			pos += dt + 1;
		}
		else {
			sl -= 2 * B;
			pos += dt;
		}
		ret[i] = fx(lpos, pos);
		lpos = pos;
	}
	assert(pos == ed);
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

std::vector<double> ChrNumThread::avg_batch( unsigned int st, unsigned int ed, unsigned int n) const {
	std::vector<double> s(sum_batch(st, ed, n));
	std::vector<unsigned int> l(coverage_batch(st, ed, n));
	unsigned int lr = (ed - st) / n;
	for (unsigned int i = 0; i < s.size(); ++i)
		s[i] = s[i] / l[i];
	return s;
}






}//namespace
