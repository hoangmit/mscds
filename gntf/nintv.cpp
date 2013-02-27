#include "nintv.h"

#include <stdexcept>
#include <utility>
#include "mem/filearchive.h"
#include "utils/param.h"

using namespace std;

namespace app_ds {

NIntvBuilder::NIntvBuilder() { clear(); }

void NIntvBuilder::clear() {
	stbd.clear();
	rlbd.clear();
	cnt = 0;
	lasted = 0;
}

void NIntvBuilder::add(size_t st, size_t ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	size_t llen = ed - st;
	if (lasted > st) throw std::runtime_error("required sorted array");
	stbd.add_inc(st);
	rlbd.add(llen);
	lasted = ed;
	++cnt;
}

void NIntvBuilder::build(NIntv *out) {
	out->len = cnt;
	stbd.build(&(out->start));
	rlbd.build(&(out->rlen));
	clear();
}

void NIntvBuilder::build(mscds::OArchive &ar) {
	NIntv a;
	build(&a);
	a.save(ar);
}

void NIntv::save(mscds::OArchive &ar) const {
	ar.startclass("non-overlapped_intervals", 1);
	ar.var("length").save(len);
	start.save(ar.var("start"));
	rlen.save(ar.var("rlen"));
	ar.endclass();
}

void NIntv::load(mscds::IArchive &ar) {
	ar.loadclass("non-overlapped_intervals");
	ar.var("length").load(len);
	start.load(ar.var("start"));
	rlen.load(ar.var("rlen"));
	ar.endclass();
}

std::pair<size_t, size_t> NIntv::find_cover(size_t pos) const {
	size_t p = rank_interval(pos);
	if (p == npos()) return pair<size_t, size_t>(0u, 0u);
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	size_t kl = pos - sp + 1;
	size_t rangelen = rlen.lookup(p);
	if (kl <= rangelen) return pair<size_t, size_t>(p, kl);
	else return pair<size_t, size_t>(p+1, 0);
}

size_t NIntv::rank_interval(size_t pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return npos();
	return p-1;
}

size_t NIntv::coverage(size_t pos) const {
	uint64_t p = rank_interval(pos);
	if (p == npos()) return 0;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	size_t ps = 0;
	size_t rangelen = rlen.lookup(p, ps);
	return std::min<size_t>((pos - sp), rangelen) + ps;
}

size_t NIntv::int_start(size_t i) const {
	return start.select(i);
}

size_t NIntv::int_len(size_t i) const {
	return rlen.lookup(i);
}

size_t NIntv::int_end(size_t i) const {
	return int_start(i) + int_len(i);
}

void NIntv::clear() {
	len = 0;
	start.clear();
	rlen.clear();
}

size_t NIntv::length() const {
	return len;
}

size_t NIntv::find_rlen(size_t val) const {
	throw std::runtime_error("not implemented");
	return 0;
}

size_t NIntv::int_psrlen(size_t i) const {
	throw std::runtime_error("not implemented");
	return 0;
}

//------------------------------------------------------------------------

NIntv2Builder::NIntv2Builder() { clear(); }

void NIntv2Builder::clear() {
	gstbd.clear();
	ilbd.clear();
	gcbd.clear();
	cnt = 0;
	last_ed = 0;
	g_pos = 0;
	llen = 0;
	first = true;
}

void NIntv2Builder::add(size_t st, size_t ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	if (first) {
		ilbd.add_inc(0);
		gcbd.add_inc(0);
		gstbd.add_inc(st);
		first = false;
		g_pos = 1;
		last_ed = ed;
	}else {
		if (last_ed > st) throw std::runtime_error("required sorted array");
		if (st > last_ed) {
			gstbd.add_inc(st);
			gcbd.add_inc(g_pos);
		}else assert(st == last_ed);
		++g_pos;
		last_ed = ed;
	}
	ilbd.add(ed - st);
	++cnt;
}

void NIntv2Builder::build(NIntv2 *out) {
	gcbd.add(g_pos);
	out->len = cnt;
	out->maxpos = last_ed;
	gstbd.build(&(out->gstart));
	gcbd.build(&(out->gcnt));
	ilbd.build(&(out->ilen));
	clear();
}

void NIntv2Builder::build(mscds::OArchive &ar) {
	NIntv2 a;
	build(&a);
	a.save(ar);
}

void NIntv2::save(mscds::OArchive &ar) const {
	ar.startclass("non-overlapped_grouped_intervals", 1);
	ar.var("length").save(len);
	ar.var("max_pos").save(maxpos);
	gstart.save(ar.var("group_start"));
	gcnt.save(ar.var("group_count"));
	ilen.save(ar.var("interval_len"));
	ar.endclass();
}

void NIntv2::load(mscds::IArchive &ar) {
	ar.loadclass("non-overlapped_grouped_intervals");
	ar.var("length").load(len);
	ar.var("max_pos").load(maxpos);
	gstart.load(ar.var("group_start"));
	gcnt.load(ar.var("group_count"));
	ilen.load(ar.var("interval_len"));
	ar.endclass();
}

std::pair<size_t, size_t> NIntv2::find_cover(size_t pos) const {
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return pair<size_t, size_t>(0, 0);
	--j;
	pos -= gstart.select(j);
	size_t ds = ilen.select(gcnt.select(j));
	size_t i = ilen.rank(pos + ds + 1) - 1;
	size_t nb = gcnt.select(j+1);
	if (i < nb) return pair<size_t, size_t>(i, pos + ds - ilen.select(i) + 1);
	else
		return pair<size_t, size_t>(nb, 0);
}

size_t NIntv2::rank_interval(size_t pos) const {
	if (pos >= maxpos) return len - 1;
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return npos();
	--j;
	pos -= gstart.select(j);
	size_t i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	size_t nb = gcnt.select(j+1);
	if (i < nb) return i;
	else return nb - 1;
}

size_t NIntv2::coverage(size_t pos) const {
	if (pos >= maxpos) return ilen.select(ilen.one_count() - 1);
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return 0;
	--j;
	pos -= gstart.select(j);
	size_t i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	if (i < gcnt.select(j+1)) return ilen.select(gcnt.select(j)) + pos;
	else return ilen.select(gcnt.select(j+1));
}

size_t NIntv2::int_start(size_t i) const {
	uint64_t j = gcnt.rank(i+1)-1;
	return gstart.select(j) + ilen.select(i) - ilen.select(gcnt.select(j));
}

size_t NIntv2::int_len(size_t i) const {
	return ilen.select(i+1) - ilen.select(i);
}

size_t NIntv2::int_end(size_t i) const {
	return int_start(i) + int_len(i);
}

void NIntv2::clear() {
	len = 0;
	gcnt.clear();
	ilen.clear();
	gstart.clear();
}

size_t NIntv2::length() const {
	return len;
}

size_t NIntv2::find_rlen(size_t val) const {
	return ilen.rank(val);
}

size_t NIntv2::int_psrlen(size_t i) const {
	return ilen.select(i);
}

//------------------------------------------------------------------------


PNIntvBuilder::PNIntvBuilder() {
	init();
}

void PNIntvBuilder::init(unsigned int _method /*= 0*/) {
	clear();
	this->method = _method;
	if (method == 0) {
		auto cf = Config::getInst();
		if (cf->hasPara("GNTF.POSITION_STORAGE")) {
			method = cf->getIntPara("GNTF.POSITION_STORAGE");
			if (method > 2) throw std::runtime_error("invalid method");
		}
	}
	autoselect = (method == 0);
	cnt = 0;
	vals.clear();
}

void PNIntvBuilder::choosemethod() {
	bd1.clear();
	bd2.clear();
	for (size_t i = 0; i < vals.size(); ++i) {
		bd1.add(vals[i].first, vals[i].second);
		bd2.add(vals[i].first, vals[i].second);
	}
	mscds::OSizeEstArchive ar;
	bd1.build(ar);
	size_t s1 = ar.opos();
	bd2.build(ar);
	size_t s2 = ar.opos() - s1;
	ar.close();
	size_t sx = s1;
	method = 1;
	if (s1 > s2) { method = 2; sx = s2; }
	for (size_t i = 0; i < vals.size(); ++i)
		addmethod(vals[i].first, vals[i].second);
}


void PNIntvBuilder::addmethod(size_t st, size_t ed) {
	if (method == 1) bd1.add(st, ed);
	else if (method == 2) bd2.add(st, ed);
	else assert(false);
}

void PNIntvBuilder::add(size_t st, size_t ed) {
	if (method == 0 && cnt <= CHECK_THRESHOLD) {
		vals.push_back(std::make_pair(st, ed));
		if (cnt == CHECK_THRESHOLD)
			choosemethod();
	} else {
		addmethod(st, ed);
	}
	cnt++;
}

void PNIntvBuilder::build(PNIntv* out) {
	out->clear();
	if (method == 0) {
		choosemethod();
	}
	out->method = method;
	out->autoselect = (int) autoselect;
	if (method == 1)
		bd1.build(&(out->m1));
	if (method == 2)
		bd2.build(&(out->m2));
}

void PNIntvBuilder::clear() {
	bd1.clear();
	bd2.clear();
	cnt = 0;
	method = 0;
}

//---

void PNIntv::save(mscds::OArchive& ar) const {
	ar.startclass("poly_non_overlap_intervals", 1);
	ar.var("method").save(method);
	ar.var("autoselect").save(autoselect);
	if (method == 1) {
		m1.save(ar.var("method1"));
	}
	else if (method == 2) {
		m2.save(ar.var("method2"));
	}
	ar.endclass();
}

void PNIntv::load( mscds::IArchive& ar ) {
	clear();
	ar.loadclass("poly_non_overlap_intervals");
	ar.var("method").load(method);
	ar.var("autoselect").load(autoselect);
	if (method == 1) {
		m1.load(ar.var("method1"));
	}
	else if (method == 2) {
		m2.load(ar.var("method2"));
	}
	ar.endclass();
}


void PNIntv::clear() {
	method = 0;
	m1.clear();
	m2.clear();
}

std::pair<size_t, size_t> PNIntv::find_cover(size_t pos) const {
	if (method == 1) return m1.find_cover(pos);
	else if (method == 2) return m2.find_cover(pos);
	else assert(false);
	return std::pair<size_t, size_t>();
}

size_t PNIntv::rank_interval(size_t pos) const {
	if (method == 1) return m1.rank_interval(pos);
	else if (method == 2) return m2.rank_interval(pos);
	else assert(false);
	return 0;
}

size_t PNIntv::find_rlen(size_t val) const {
	if (method == 1) return m1.find_rlen(val);
	else if (method == 2) return m2.find_rlen(val);
	else assert(false);
	return 0;
}

size_t PNIntv::int_start(size_t i) const {
	if (method == 1) return m1.int_start(i);
	else if (method == 2) return m2.int_start(i);
	else assert(false);
	return 0;
}

size_t PNIntv::int_len(size_t i) const {
	if (method == 1) return m1.int_len(i);
	else if (method == 2) return m2.int_len(i);
	else assert(false);
	return 0;
}

size_t PNIntv::int_end(size_t i) const {
	if (method == 1) return m1.int_end(i);
	else if (method == 2) return m2.int_end(i);
	else assert(false);
	return 0;
}

size_t PNIntv::int_psrlen(size_t i) const {
	if (method == 1) return m1.int_psrlen(i);
	else if (method == 2) return m2.int_psrlen(i);
	else assert(false);
	return 0;
}

size_t PNIntv::length() const {
	if (method == 1) return m1.length();
	else if (method == 2) return m2.length();
	else assert(false);
	return 0;
}


} // namespace
