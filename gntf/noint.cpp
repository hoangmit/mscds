#include "noint.h"

#include <stdexcept>
#include <utility>
#include "mem/filearchive.h"
#include "utils/param.h"

using namespace std;

namespace app_ds {

NOIntBuilder::NOIntBuilder() { clear(); }

void NOIntBuilder::clear() {
	stbd.clear();
	rlbd.clear();
	cnt = 0;
	lasted = 0;
}

void NOIntBuilder::add(size_t st, size_t ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	size_t llen = ed - st;
	if (lasted > st) throw std::runtime_error("required sorted array");
	stbd.add_inc(st);
	rlbd.add(llen);
	lasted = ed;
	++cnt;
}

void NOIntBuilder::build(NOInt *out) {
	out->len = cnt;
	stbd.build(&(out->start));
	rlbd.build(&(out->rlen));
	clear();
}

void NOIntBuilder::build(mscds::OArchive &ar) {
	NOInt a;
	build(&a);
	a.save(ar);
}

void NOInt::save(mscds::OArchive &ar) const {
	ar.startclass("non-overlapped_intervals", 1);
	ar.var("length").save(len);
	start.save(ar.var("start"));
	rlen.save(ar.var("rlen"));
	ar.endclass();
}

void NOInt::load(mscds::IArchive &ar) {
	ar.loadclass("non-overlapped_intervals");
	ar.var("length").load(len);
	start.load(ar.var("start"));
	rlen.load(ar.var("rlen"));
	ar.endclass();
}

std::pair<size_t, bool> NOInt::find_interval(size_t pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return std::pair<size_t, bool>(0, false);
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	size_t kl = pos - sp;
	size_t rangelen = rlen.lookup(p);
	if (rangelen > kl) return std::pair<size_t, bool>(p, true);
	else return std::pair<size_t, bool>(p+1, false);
}

size_t NOInt::coverage(size_t pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return 0;
	p--;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	size_t ps = 0;
	size_t rangelen = rlen.lookup(p, ps);
	return std::min<size_t>((pos - sp), rangelen) + ps;
}

size_t NOInt::int_start(size_t i) const {
	return start.select(i);
}

size_t NOInt::int_len(size_t i) const {
	return rlen.lookup(i);
}

size_t NOInt::int_end(size_t i) const {
	return int_start(i) + int_len(i);
}

void NOInt::clear() {
	len = 0;
	start.clear();
	rlen.clear();
}

size_t NOInt::length() const {
	return len;
}

size_t NOInt::find_rlen(size_t val) const {
	throw std::runtime_error("not implemented");
	return 0;
}

size_t NOInt::int_psrlen(size_t i) const {
	throw std::runtime_error("not implemented");
	return 0;
}

//------------------------------------------------------------------------

NOInt2Builder::NOInt2Builder() { clear(); }

void NOInt2Builder::clear() {
	gstbd.clear();
	ilbd.clear();
	gcbd.clear();
	cnt = 0;
	last_ed = 0;
	g_cnt = 0;
	llen = 0;
}

void NOInt2Builder::add(size_t st, size_t ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	if (last_ed > st) throw std::runtime_error("required sorted array");
	if (ed > last_ed) {
		gstbd.add_inc(st);
		gcbd.add_inc(g_cnt);
	} else
		assert(ed == last_ed);
	ilbd.add_inc(llen);
	++g_cnt;
	last_ed = ed;
	llen += ed - st;
	++cnt;
}

void NOInt2Builder::build(NOInt2 *out) {
	gcbd.add(g_cnt);
	ilbd.add_inc(llen);
	out->len = cnt;
	out->maxpos = last_ed;
	gstbd.build(&(out->gstart));
	gcbd.build(&(out->gcnt));
	ilbd.build(&(out->ilen));
	clear();
}

void NOInt2Builder::build(mscds::OArchive &ar) {
	NOInt2 a;
	build(&a);
	a.save(ar);
}

void NOInt2::save(mscds::OArchive &ar) const {
	ar.startclass("non-overlapped_grouped_intervals", 1);
	ar.var("length").save(len);
	ar.var("max_pos").save(maxpos);
	gstart.save(ar.var("group_start"));
	gcnt.save(ar.var("group_count"));
	ilen.save(ar.var("interval_len"));
	ar.endclass();
}

void NOInt2::load(mscds::IArchive &ar) {
	ar.loadclass("non-overlapped_grouped_intervals");
	ar.var("length").load(len);
	ar.var("max_pos").load(maxpos);
	gstart.load(ar.var("group_start"));
	gcnt.load(ar.var("group_count"));
	ilen.load(ar.var("interval_len"));
	ar.endclass();
}

std::pair<size_t, bool> NOInt2::find_interval(size_t pos) const {
	if (pos >= maxpos) return std::pair<size_t, bool>(length(), false);
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return std::pair<size_t, bool>(0, false);
	--j;
	pos -= gstart.select(j);
	size_t i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	if (i < gcnt.select(j+1)) return std::pair<size_t, bool>(i, true);
	else return std::pair<size_t, bool>(gcnt.select(j+1), false);
}

size_t NOInt2::coverage(size_t pos) const {
	if (pos >= maxpos) return ilen.select(ilen.one_count() - 1);
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return 0;
	--j;
	pos -= gstart.select(j);
	size_t i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	if (i < gcnt.select(j+1)) return ilen.select(gcnt.select(j)) + pos;
	else return ilen.select(gcnt.select(j+1));
}

size_t NOInt2::int_start(size_t i) const {
	uint64_t j = gcnt.rank(i);
	return gstart.select(j) + ilen.select(i) - ilen.select(gcnt.select(j));
}

size_t NOInt2::int_len(size_t i) const {
	return ilen.select(i+1) - ilen.select(i);
}

size_t NOInt2::int_end(size_t i) const {
	return int_start(i) + int_len(i);
}

void NOInt2::clear() {
	len = 0;
	gcnt.clear();
	ilen.clear();
	gstart.clear();
}

size_t NOInt2::length() const {
	return len;
}

size_t NOInt2::find_rlen( size_t val ) const {
	throw std::runtime_error("not implemented");
	return 0;
}

size_t NOInt2::int_psrlen( size_t i ) const {
	throw std::runtime_error("not implemented");
	return 0;
}

//------------------------------------------------------------------------


PNOIntBuilder::PNOIntBuilder() {
	init();
}

void PNOIntBuilder::init(unsigned int _method /*= 0*/) {
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

void PNOIntBuilder::choosemethod() {
	bd1.clear();
	bd2.clear();
	for (size_t i = 0; i < vals.size(); ++i) {
		bd1.add(vals[i].first, vals[i].second);
		bd2.add(vals[i].first, vals[i].second);
	}
	mscds::OClassInfoArchive ar;
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


void PNOIntBuilder::addmethod(size_t st, size_t ed) {
	if (method == 1) bd1.add(st, ed);
	else if (method == 2) bd2.add(st, ed);
	else assert(false);
}

void PNOIntBuilder::add(size_t st, size_t ed) {
	if (method == 0 && cnt <= CHECK_THRESHOLD) {
		vals.push_back(std::make_pair(st, ed));
		if (cnt == CHECK_THRESHOLD)
			choosemethod();
	} else {
		addmethod(st, ed);
	}
	cnt++;
}

void PNOIntBuilder::build(PNOInt* out) {
	out->clear();
	assert(method != 0);
	out->method = method;
	out->autoselect = (int) autoselect;
	bd1.build(&(out->m1));
	bd2.build(&(out->m2));
}

void PNOIntBuilder::clear() {
	bd1.clear();
	bd2.clear();
	cnt = 0;
	method = 0;
}

//---

void PNOInt::save(mscds::OArchive& ar) const {
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

void PNOInt::load( mscds::IArchive& ar ) {
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


void PNOInt::clear() {
	method = 0;
	m1.clear();
	m2.clear();
}

std::pair<size_t, bool> PNOInt::find_interval(size_t pos) const {
	if (method == 1) return m1.find_interval(pos);
	else if (method == 2) return m2.find_interval(pos);
	else assert(false);
	return make_pair(0,false);
}

size_t PNOInt::find_rlen(size_t val) const {
	if (method == 1) return m1.find_rlen(val);
	else if (method == 2) return m2.find_rlen(val);
	else assert(false);
	return 0;
}

size_t PNOInt::int_start(size_t i) const {
	if (method == 1) return m1.int_start(i);
	else if (method == 2) return m2.int_start(i);
	else assert(false);
	return 0;
}

size_t PNOInt::int_len(size_t i) const {
	if (method == 1) return m1.int_len(i);
	else if (method == 2) return m2.int_len(i);
	else assert(false);
	return 0;
}

size_t PNOInt::int_end(size_t i) const {
	if (method == 1) return m1.int_end(i);
	else if (method == 2) return m2.int_end(i);
	else assert(false);
	return 0;
}

size_t PNOInt::int_psrlen(size_t i) const {
	if (method == 1) return m1.int_psrlen(i);
	else if (method == 2) return m2.int_psrlen(i);
	else assert(false);
	return 0;
}

size_t PNOInt::length() const {
	if (method == 1) return m1.length();
	else if (method == 2) return m2.length();
	else assert(false);
	return 0;
}

} // namespace
