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

void NIntvBuilder::add(PosType st, PosType ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	PosType llen = ed - st;
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

std::pair<NIntv::PosType, NIntv::PosType> NIntv::find_cover(PosType pos) const {
	PosType p = rank_interval(pos);
	if (p == npos()) return pair<PosType, PosType>(0u, 0u);
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	PosType kl = pos - sp + 1;
	PosType rangelen = rlen.lookup(p);
	if (kl <= rangelen) return pair<PosType, PosType>(p, kl);
	else return pair<PosType, PosType>(p+1, 0);
}

NIntv::PosType NIntv::rank_interval(PosType pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return npos();
	return p-1;
}

NIntv::PosType NIntv::coverage(PosType pos) const {
	uint64_t p = rank_interval(pos);
	if (p == npos()) return 0;
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	uint64_t ps = 0;
	PosType rangelen = rlen.lookup(p, ps);
	return std::min<PosType>((pos - sp), rangelen) + ps;
}

NIntv::PosType NIntv::int_start(PosType i) const {
	return start.select(i);
}

NIntv::PosType NIntv::int_len(PosType i) const {
	return rlen.lookup(i);
}

NIntv::PosType NIntv::int_end(PosType i) const {
	return int_start(i) + int_len(i);
}

void NIntv::clear() {
	len = 0;
	start.clear();
	rlen.clear();
}

NIntv::PosType NIntv::length() const {
	return len;
}

NIntv::PosType NIntv::find_rlen(PosType val) const {
	throw std::runtime_error("not implemented");
	return 0;
}

NIntv::PosType NIntv::int_psrlen(PosType i) const {
	return rlen.prefixsum(i);
}

void NIntv::getEnum(PosType idx, Enum * e) const {
	rlen.getEnum(idx, &(e->re));
	start.getEnum(idx, &(e->st));
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

void NIntv2Builder::add(PosType st, PosType ed) {
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
		}else { assert(st == last_ed); }
		++g_pos;
		last_ed = ed;
	}
	ilbd.add(ed - st);
	++cnt;
}

void NIntv2Builder::build(NIntv2 *out) {
	gcbd.add_inc(g_pos);
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

std::pair<NIntv2::PosType, NIntv2::PosType> NIntv2::find_cover(PosType pos) const {
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return pair<PosType, PosType>(0, 0);
	--j;
	pos -= gstart.select(j);
	PosType ds = ilen.select(gcnt.select(j));
	PosType i = ilen.rank(pos + ds + 1) - 1;
	PosType nb = gcnt.select(j+1);
	if (i < nb) return pair<PosType, PosType>(i, pos + ds - ilen.select(i) + 1);
	else
		return pair<PosType, PosType>(nb, 0);
}

NIntv2::PosType NIntv2::rank_interval(PosType pos) const {
	if (pos >= maxpos) return len - 1;
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return npos();
	--j;
	pos -= gstart.select(j);
	PosType i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	PosType nb = gcnt.select(j+1);
	if (i < nb) return i;
	else return nb - 1;
}

NIntv2::PosType NIntv2::coverage(PosType pos) const {
	if (pos >= maxpos) return ilen.select(ilen.one_count() - 1);
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return 0;
	--j;
	pos -= gstart.select(j);
	PosType i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	if (i < gcnt.select(j+1)) return ilen.select(gcnt.select(j)) + pos;
	else return ilen.select(gcnt.select(j+1));
}

NIntv2::PosType NIntv2::int_start(PosType i) const {
	uint64_t j = gcnt.rank(i+1)-1;
	return gstart.select(j) + ilen.select(i) - ilen.select(gcnt.select(j));
}

NIntv2::PosType NIntv2::int_len(PosType i) const {
	return ilen.select(i+1) - ilen.select(i);
}

NIntv2::PosType NIntv2::int_end(PosType i) const {
	return int_start(i) + int_len(i);
}

void NIntv2::clear() {
	len = 0;
	gcnt.clear();
	ilen.clear();
	gstart.clear();
}

NIntv2::PosType NIntv2::length() const {
	return len;
}

NIntv2::PosType NIntv2::find_rlen(PosType val) const {
	return ilen.rank(val);
}

NIntv2::PosType NIntv2::int_psrlen(PosType i) const {
	return ilen.select(i);
}

void NIntv2::getEnum(PosType idx, Enum *e) const {
	ilen.getDisEnum(idx+1, &(e->rl));
	auto j = gcnt.rank(idx+1);
	gcnt.getDisEnum(j, &(e->gc));
	e->gi = e->gc.next() + gcnt.select(j-1) - idx;
	e->cp = int_start(idx);
	gstart.getEnum(j, &(e->gs));
}

void NIntv2::inspect(const std::string& cmd, std::ostream& out) const {
	out << gcnt.to_str() << "\n";
}


std::pair<NIntv2::PosType, NIntv2::PosType> NIntv2::Enum::next() {
	NIntv2::PosType xp = cp;
	NIntv2::PosType xxp = cp + rl.next();
	gi--;
	if (gi == 0 && gc.hasNext()) {
		gi = gc.next();
		cp = gs.next();
	}else
		cp = xxp;
	return std::pair<NIntv2::PosType, NIntv2::PosType>(xp, xxp);
}

bool NIntv2::Enum::hasNext() const {
	return rl.hasNext();
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
	for (PosType i = 0; i < vals.size(); ++i) {
		bd1.add(vals[i].first, vals[i].second);
		bd2.add(vals[i].first, vals[i].second);
	}
	mscds::OSizeEstArchive ar;
	bd1.build(ar);
	PosType s1 = ar.opos();
	bd2.build(ar);
	PosType s2 = ar.opos() - s1;
	ar.close();
	PosType sx = s1;
	method = 1;
	if (s1 > s2) { method = 2; sx = s2; }
	for (PosType i = 0; i < vals.size(); ++i)
		addmethod(vals[i].first, vals[i].second);
}


void PNIntvBuilder::addmethod(PosType st, PosType ed) {
	if (method == 1) bd1.add(st, ed);
	else if (method == 2) bd2.add(st, ed);
	else assert(false);
}

void PNIntvBuilder::add(PosType st, PosType ed) {
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
	} else if (method == 2) {
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

std::pair<PNIntv::PosType, PNIntv::PosType> PNIntv::find_cover(PosType pos) const {
	if (method == 1) return m1.find_cover(pos);
	else if (method == 2) return m2.find_cover(pos);
	else assert(false);
	return std::pair<PosType, PosType>();
}

PNIntv::PosType PNIntv::rank_interval(PosType pos) const {
	if (method == 1) return m1.rank_interval(pos);
	else if (method == 2) return m2.rank_interval(pos);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::find_rlen(PosType val) const {
	if (method == 1) return m1.find_rlen(val);
	else if (method == 2) return m2.find_rlen(val);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::coverage(PosType pos) const {
	if (method == 1) return m1.coverage(pos);
	else if (method == 2) return m2.coverage(pos);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_start(PosType i) const {
	if (method == 1) return m1.int_start(i);
	else if (method == 2) return m2.int_start(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_len(PosType i) const {
	if (method == 1) return m1.int_len(i);
	else if (method == 2) return m2.int_len(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_end(PosType i) const {
	if (method == 1) return m1.int_end(i);
	else if (method == 2) return m2.int_end(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_psrlen(PosType i) const {
	if (method == 1) return m1.int_psrlen(i);
	else if (method == 2) return m2.int_psrlen(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::length() const {
	if (method == 1) return m1.length();
	else if (method == 2) return m2.length();
	else assert(false);
	return 0;
}

void PNIntv::getEnum( PosType idx, Enum *e ) const {
	e->method = method;
	if (method == 1) m1.getEnum(idx, &(e->e1));
	else
		if (method == 2) m2.getEnum(idx, &(e->e2));
		else throw std::runtime_error("not initilized");
}

void PNIntv::inspect( const std::string& cmd, std::ostream& out ) const {
	if (method == 1) m1.inspect(cmd, out);
	else if (method == 2) m2.inspect(cmd, out);
}



std::pair<NIntv::PosType, NIntv::PosType> NIntv::Enum::next() {
	auto stx = st.next();
	return std::pair<PosType, PosType>(stx, stx + re.next());
}


} // namespace
