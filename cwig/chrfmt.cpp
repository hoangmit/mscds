#include "chrfmt.h"
#include "mem/filearchive.h"
#include "utils/modp_numtoa.h"

#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <limits>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>

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
	for (unsigned int i = 1; i < ranges.size(); ++i)
	if (ranges[i - 1].ed > ranges[i].st)
		throw std::runtime_error("input contains overlapping intervals");
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
	double sx = sum(st, ed);
	unsigned int cov = coverage(st, ed);
	double sqsx = sqrsum(st, ed);
	double varance = (sqsx - sx * sx / cov) / cov;
	return sqrt(varance);
}

double ChrNumThread::min_value(unsigned int st, unsigned int ed) const {
	if (st >= ed) throw std::runtime_error("invalid input interval");
	if (minmax_opt & MIN_OP) {
		auto ls = vals.find_intervals(st, ed);
		if (ls.first < ls.second) {
			unsigned int i = min.m_idx(ls.first, ls.second);
			return vals.range_value(i);
		}
		else return 0;
	}
	else return 0;
}

double ChrNumThread::max_value(unsigned int st, unsigned int ed) const {
	if (st >= ed) throw std::runtime_error("invalid input interval");
	if (minmax_opt & MAX_OP) {
		auto ls = vals.find_intervals(st, ed);
		if (ls.first < ls.second) {
			unsigned int i = max.m_idx(ls.first, ls.second);
			return vals.range_value(i);
		}
		else return 0;
	}
	else return 0;
}

std::vector<double> ChrNumThread::base_value_map(unsigned int st, unsigned int ed) const {
	if (st >= ed) throw runtime_error("wrong function inputs");
	vector<double> out(ed - st, std::numeric_limits<double>::quiet_NaN());
	auto rng = find_intervals(st, ed);
	if (rng.first < rng.second) {
		app_ds::ChrNumValType::Enum e;
		getEnum(rng.first, &e);
		for (unsigned int i = rng.first; i < rng.second; ++i) {
			auto intv = e.next();
			unsigned int pst = (intv.st >= st) ? intv.st - st : 0;
			unsigned int ped = (intv.ed <= ed) ? intv.ed - st : ed - st;
			for (unsigned int j = pst; j < ped; ++j)
				out[j] = intv.val;
		}
	}
	return out;
}

template<typename Func>
void endpoints(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	//assert(ed - st >= n);
	//assert(n > 0);
	if (n == 0 || st >= ed) throw runtime_error("wrong function inputs");
	unsigned int l = (ed - st), dt = l / n, r = l % n;
	int A = n - r, B = r;
	int sl = 0;
	unsigned int pos = st;
	for (int i = 0; i < n; ++i) {
		if (sl + A <= B) {
			sl += 2 * A;
			pos += dt + 1;
		} else {
			sl -= 2 * B;
			pos += dt;
		}
		fx(i, pos);
	}
	assert(ed == pos);
}

template<typename Tp, typename Func>
std::vector<Tp> mapValue(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	vector<Tp> out(n);
	unsigned int last = 0;
	endpoints(0, n, ed - st, [&](unsigned int i, unsigned pos) {
		for (unsigned int j = last; j < pos; ++j) out[j] = fx(i);
		last = pos;
	});
	return out;
}

template<typename Tp, typename Func>
inline std::vector<Tp> diff_func(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	vector<Tp> out(n);
	Tp lastval = fx(st);
	endpoints(st, ed, n, [&](unsigned int i, unsigned pos) {
		Tp cv = fx(pos); out[i] = cv - lastval; lastval = cv; });
	return out;
}

std::vector<double> ChrNumThread::sum_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n) return diff_func<double>(st, ed, n, [&](unsigned int pos)->double{
		return this->sum(pos); });
	else {
		auto arr = base_value_map(st, ed);
		return mapValue<double>(st, ed, n, [&](double i)->double{return arr[i]; });
	}
}

std::vector<unsigned int> ChrNumThread::count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n) return diff_func<unsigned int>(st, ed, n, [&](unsigned int pos)->double{
		return this->count_intervals(pos); });
	else {
		auto arr = base_value_map(st, ed);
		return mapValue<unsigned int>(st, ed, n, [&](double i)->unsigned int{
			return boost::math::isnan(arr[i]) ? 0 : 1 ; });
	}
}

std::vector<unsigned int> ChrNumThread::coverage_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n) return diff_func<unsigned int>(st, ed, n, [&](unsigned int pos)->unsigned int{
		return this->coverage(pos); });
	else {
		auto arr = base_value_map(st, ed);
		return mapValue<unsigned int>(st, ed, n, [&](double i)->unsigned int{
			return boost::math::isnan(arr[i]) ? 0 : 1; });
	}
}

std::vector<double> ChrNumThread::avg_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n) {
		std::vector<double> ret(n);
		pair<double, double> last = make_pair(this->sum(st), this->coverage(st));
		endpoints(st, ed, n, [&](unsigned int i, unsigned pos) {
			pair<double, double> cv = make_pair(this->sum(pos), this->coverage(pos));
			ret[i] = (cv.first - last.first) / (cv.second - last.second);
			last = cv;
		});
		return ret;
	} else {
		auto arr = base_value_map(st, ed);
		return mapValue<double>(st, ed, n, [&](double i)->double{return arr[i]; });
	}
}

template<typename Tp, typename Func>
inline std::vector<Tp> range_summary(unsigned int st, unsigned int ed, unsigned int n, Func fx) {
	unsigned int last = st;
	vector<Tp> out(n);
	endpoints(st, ed, n, [&](unsigned int i, unsigned pos) { out[i] = fx(last, pos); last = pos; });
	return out;
}

std::vector<double> ChrNumThread::min_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n)
		return range_summary<double>(st, ed, n, [&](unsigned int st, unsigned int ed)->double{
			return this->min_value(st, ed); });
	else {
		auto arr = base_value_map(st, ed);
		return mapValue<double>(st, ed, n, [&](double i)->double{return arr[i]; });
	}
}

std::vector<double> ChrNumThread::max_value_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n)
		return range_summary<double>(st, ed, n, [&](unsigned int st, unsigned int ed)->double{
			return this->max_value(st, ed); });
	else {
		auto arr = base_value_map(st, ed);
		return mapValue<double>(st, ed, n, [&](double i)->double{return arr[i]; });
	}
}

std::vector<double> ChrNumThread::stdev_batch(unsigned int st, unsigned int ed, unsigned int n) const {
	if (ed - st > n)
		return range_summary<double>(st, ed, n, [&](unsigned int st, unsigned int ed)->double{
			return this->stdev(st, ed); });
	else {
		return mapValue<double>(st, ed, n, [&](double i)->double{return 0; });
	}
}

}//namespace
