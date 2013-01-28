#include "noint.h"

#include <stdexcept>

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

} // namespace
