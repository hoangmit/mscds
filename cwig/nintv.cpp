#include "nintv.h"

#include <stdexcept>
#include <utility>
#include <algorithm>

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
	if (lasted > st) throw std::runtime_error("overlapping intervals");
	stbd.add_inc(st);
	rlbd.add(llen);
	lasted = ed;
	++cnt;
}

void NIntvBuilder::build(NIntv *out) {
	out->clear();
	out->len = cnt;
	stbd.build(&(out->start));
	rlbd.build(&(out->rlen));
	clear();
}

void NIntvBuilder::build(mscds::OutArchive &ar) {
	NIntv a;
	build(&a);
	a.save(ar);
}

void NIntv::save(mscds::OutArchive &ar) const {
	ar.startclass("non-overlapped_intervals", 1);
	ar.var("length").save(len);
	start.save(ar.var("start"));
	rlen.save(ar.var("rlen"));
	ar.endclass();
}

void NIntv::load(mscds::InpArchive &ar) {
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

std::pair<NIntv::PosType, NIntv::PosType> NIntv::int_startend(NIntv::PosType i) const {
	NIntv::PosType st = start.select(i);
	return make_pair(st, st + int_len(i));
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

std::pair<NIntv::PosType, NIntv::PosType> NIntv::Enum::next() {
	auto stx = st.next();
	return std::pair<PosType, PosType>(stx, stx + re.next());
}


//------------------------------------------------------------------------

NIntvGroupBuilder::NIntvGroupBuilder() { clear(); }

void NIntvGroupBuilder::clear() {
	gstbd.clear();
	ilbd.clear();
	gcbd.clear();
	cnt = 0;
	last_ed = 0;
	g_pos = 0;
	llen = 0;
	first = true;
}

void NIntvGroupBuilder::add(PosType st, PosType ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	if (first) {
		ilbd.add_inc(0);
		gcbd.add_inc(0);
		gstbd.add_inc(st);
		first = false;
		g_pos = 1;
		last_ed = ed;
	}else {
		if (last_ed > st) throw std::runtime_error("overlapping intervals");
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

void NIntvGroupBuilder::build(NIntvGroup *out) {
	out->clear();
	gcbd.add_inc(g_pos);
	out->len = cnt;
	out->maxpos = last_ed;
	gstbd.build(&(out->gstart));
	gcbd.build(&(out->gcnt));
	ilbd.build(&(out->ilen));
	clear();
}

void NIntvGroupBuilder::build(mscds::OutArchive &ar) {
	NIntvGroup a;
	build(&a);
	a.save(ar);
}

void NIntvGroup::save(mscds::OutArchive &ar) const {
	ar.startclass("non-overlapped_grouped_intervals", 1);
	ar.var("length").save(len);
	ar.var("max_pos").save(maxpos);
	gstart.save(ar.var("group_start"));
	gcnt.save(ar.var("group_count"));
	ilen.save(ar.var("interval_len"));
	ar.endclass();
}

void NIntvGroup::load(mscds::InpArchive &ar) {
	ar.loadclass("non-overlapped_grouped_intervals");
	ar.var("length").load(len);
	ar.var("max_pos").load(maxpos);
	gstart.load(ar.var("group_start"));
	gcnt.load(ar.var("group_count"));
	ilen.load(ar.var("interval_len"));
	ar.endclass();
}

std::pair<NIntvGroup::PosType, NIntvGroup::PosType> NIntvGroup::find_cover(PosType pos) const {
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

NIntvGroup::PosType NIntvGroup::rank_interval(PosType pos) const {
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

NIntvGroup::PosType NIntvGroup::coverage(PosType pos) const {
	if (pos >= maxpos) return ilen.select(ilen.one_count() - 1);
	uint64_t j = gstart.rank(pos+1);
	if (j == 0) return 0;
	--j;
	pos -= gstart.select(j);
	PosType i = ilen.rank(pos + ilen.select(gcnt.select(j)) + 1) - 1;
	if (i < gcnt.select(j+1)) return ilen.select(gcnt.select(j)) + pos;
	else return ilen.select(gcnt.select(j+1));
}

NIntvGroup::PosType NIntvGroup::int_start(PosType i) const {
	uint64_t j = gcnt.rank(i+1)-1;
	return gstart.select(j) + ilen.select(i) - ilen.select(gcnt.select(j));
}

NIntvGroup::PosType NIntvGroup::int_len(PosType i) const {
	NIntvGroup::PosType ret = ilen.sdarray().lookup(i + 1);
	assert(ret == ilen.select(i+1) - ilen.select(i));
	return ret;
}

NIntvGroup::PosType NIntvGroup::int_end(PosType i) const {
	return int_start(i) + int_len(i);
}

std::pair<NIntvGroup::PosType, NIntvGroup::PosType> NIntvGroup::int_startend(NIntvGroup::PosType i) const {
	uint64_t j = gcnt.rank(i + 1) - 1;
	uint64_t sli = ilen.select(i);
	uint64_t st = gstart.select(j) + sli - ilen.select(gcnt.select(j));
	return std::pair<PosType, PosType>(st, st + ilen.select(i + 1) - sli);
}

void NIntvGroup::clear() {
	len = 0;
	gcnt.clear();
	ilen.clear();
	gstart.clear();
}

NIntvGroup::PosType NIntvGroup::length() const {
	return len;
}

NIntvGroup::PosType NIntvGroup::find_rlen(PosType val) const {
	return ilen.rank(val);
}

NIntvGroup::PosType NIntvGroup::int_psrlen(PosType i) const {
	return ilen.select(i);
}

void NIntvGroup::getEnum(PosType idx, Enum *e) const {
	ilen.getDisEnum(idx+1, &(e->rl));
	auto j = gcnt.rank(idx+1);
	gcnt.getDisEnum(j, &(e->gc));
	e->gi = e->gc.next() + gcnt.select(j-1) - idx;
	e->cp = int_start(idx);
	gstart.getEnum(j, &(e->gs));
}

void NIntvGroup::inspect(const std::string& cmd, std::ostream& out) const {
	out << gcnt.to_str() << "\n";
}


std::pair<NIntvGroup::PosType, NIntvGroup::PosType> NIntvGroup::Enum::next() {
	NIntvGroup::PosType xp = cp;
	NIntvGroup::PosType xxp = cp + rl.next();
	gi--;
	if (gi == 0 && gc.hasNext()) {
		gi = gc.next();
		cp = gs.next();
	}else
		cp = xxp;
	return std::pair<NIntvGroup::PosType, NIntvGroup::PosType>(xp, xxp);
}

bool NIntvGroup::Enum::hasNext() const {
	return rl.hasNext();
}

//------------------------------------------------------------------------

NIntvGapBuilder::NIntvGapBuilder() { clear(); }

void NIntvGapBuilder::add(NIntvGapBuilder::PosType st, NIntvGapBuilder::PosType ed) {
	if (ed <= st) throw std::runtime_error("invalid range");
	PosType llen = ed - st;
	if (lasted > st) throw std::runtime_error("overlapping intervals");
	stbd.add_inc(st);
	rgapbd.add(st - lasted);
	lasted = ed;
	++cnt;
}

void NIntvGapBuilder::build(NIntvGap *out) {
	out->clear();
	stbd.add_inc(lasted);
	rgapbd.add(0);
	out->len = cnt;
	stbd.build(&out->start);
	rgapbd.build(&out->rgap);
	clear();
}

void NIntvGapBuilder::clear() {
	stbd.clear();
	rgapbd.clear();
	cnt = 0;
	lasted = 0;
}

void NIntvGap::save(mscds::OutArchive &ar) const {
	ar.startclass("non-overlapped_intervals_gap", 1);
	ar.var("length").save(len);
	start.save(ar.var("start"));
	rgap.save(ar.var("rlen"));
	ar.endclass();
}

void NIntvGap::load(mscds::InpArchive &ar) {
	ar.loadclass("non-overlapped_intervals_gap");
	ar.var("length").load(len);
	start.load(ar.var("start"));
	rgap.load(ar.var("rlen"));
	ar.endclass();
}

NIntvGap::PosType NIntvGap::rank_interval(NIntvGap::PosType pos) const {
	uint64_t p = start.rank(pos+1);
	if (p == 0) return npos();
	else
		if (p < len) return p-1;
		else return len - 1;
}

std::pair<NIntvGap::PosType, NIntvGap::PosType> NIntvGap::find_cover(NIntvGap::PosType pos) const {
	PosType p = rank_interval(pos);
	if (p == npos()) return pair<PosType, PosType>(0u, 0u);
	uint64_t sp = start.select(p);
	assert(sp <= pos);
	PosType kl = pos - sp + 1;
	PosType rangelen = int_len(p);
	if (kl <= rangelen) return pair<PosType, PosType>(p, kl);
	else return pair<PosType, PosType>(p+1, 0);
}

NIntvGap::PosType NIntvGap::coverage(NIntvGap::PosType pos) const {
	uint64_t p = rank_interval(pos);
	if (p == npos()) return 0;
	uint64_t pre;
	uint64_t diff = start.sdarray().lookup(p + 1, pre);
	uint64_t gapsum;
	PosType gaplen = rgap.lookup(p + 1, gapsum);
	uint64_t len = diff - gaplen;
	if (pos - pre >= len) return pre - gapsum + len;
	else return pre - gapsum + pos - pre;
}

NIntvGap::PosType NIntvGap::int_start(PosType i) const {
	assert(i < len);
	return start.select(i);
}

NIntvGap::PosType NIntvGap::int_len(PosType i) const {
	assert(i < len);
	uint64_t diff = start.sdarray().lookup(i + 1);
	assert(diff == start.select(i+1) - start.select(i));
	return diff - rgap.lookup(i + 1);
}

NIntvGap::PosType NIntvGap::int_end(PosType i) const {
	assert(i < len);
	uint64_t next_st = start.select(i+1);
	return next_st - rgap.lookup(i + 1);
}

std::pair<NIntvGap::PosType, NIntvGap::PosType> NIntvGap::int_startend(NIntvGap::PosType i) const {
	assert(i < len);
	uint64_t pre;
	uint64_t diff = start.sdarray().lookup(i + 1, pre);
	return std::pair<PosType, PosType>(pre, pre + diff - rgap.lookup(i + 1));
}

void NIntvGap::clear() {
	len = 0;
	start.clear();
	rgap.clear();
}

NIntvGap::PosType NIntvGap::length() const {
	return len;
}

NIntvGap::PosType NIntvGap::find_rlen(PosType val) const {
	throw std::runtime_error("not implemented");
	return 0;
}

NIntvGap::PosType NIntvGap::int_psrlen(PosType i) const {
	if (i == 0) return 0;
	else return start.select(i) - rgap.prefixsum(i + 1);
}

void NIntvGap::getEnum(PosType idx, Enum * e) const {
	start.getEnum(idx, &(e->st));
	assert(e->st.hasNext());
	e->last = e->st.next();
	rgap.getEnum(idx + 1, &(e->rg));
}

std::pair<NIntvGap::PosType, NIntvGap::PosType> NIntvGap::Enum::next() {
	auto stx = st.next();
	std::pair<PosType, PosType> ret(last, stx - rg.next());
	last = stx;
	return ret;
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
		if (cf->check("CWIG.POSITION_STORAGE")) {
			method = cf->getInt("CWIG.POSITION_STORAGE");
			if (method > 3) throw std::runtime_error("invalid method");
		}
	}
	autoselect = (method == 0);
	cnt = 0;
	vals.clear();
}

void PNIntvBuilder::choosemethod() {
	unsigned int contcnt = 0;
	for (PosType i = 1; i < vals.size(); ++i) {
		if (vals[i - 1].second == vals[i].first)
			++contcnt;
	}
	if (2 * contcnt > vals.size()) method = 3;
	else method = 1;
	for (PosType i = 0; i < vals.size(); ++i)
		addmethod(vals[i].first, vals[i].second);
}


void PNIntvBuilder::addmethod(PosType st, PosType ed) {
	if (method == 1) bd1.add(st, ed);
	else if (method == 2) bd2.add(st, ed);
	else if (method == 3) bd3.add(st, ed);
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
	else
	if (method == 2)
		bd2.build(&(out->m2));
	else
	if (method == 3)
		bd3.build(&(out->m3));
	else assert(false);
}

void PNIntvBuilder::clear() {
	bd1.clear();
	bd2.clear();
	bd3.clear();
	cnt = 0;
	method = 0;
}

//-------------------------------------------------------

void PNIntv::save(mscds::OutArchive& ar) const {
	ar.startclass("poly_non_overlap_intervals", 1);
	ar.var("method").save(method);
	ar.var("autoselect").save(autoselect);
	if (method == 1) {
		m1.save(ar.var("method1"));
	} else if (method == 2) {
		m2.save(ar.var("method2"));
	}
	else if (method == 3) {
		m3.save(ar.var("method3"));
	}
	else { assert(false); }
	ar.endclass();
}

void PNIntv::load( mscds::InpArchive& ar ) {
	clear();
	ar.loadclass("poly_non_overlap_intervals");
	ar.var("method").load(method);
	ar.var("autoselect").load(autoselect);
	if (method == 1) {
		m1.load(ar.var("method1"));
	} else if (method == 2) {
		m2.load(ar.var("method2"));
	}
	else if (method == 3) {
		m3.load(ar.var("method3"));
	}
	else {
		assert(false);
	}
	ar.endclass();
}

void PNIntv::clear() {
	method = 0;
	autoselect = false;
	m1.clear();
	m2.clear();
	m3.clear();
}

std::pair<PNIntv::PosType, PNIntv::PosType> PNIntv::find_cover(PosType pos) const {
	if (method == 1) return m1.find_cover(pos);
	else if (method == 2) return m2.find_cover(pos);
	else if (method == 3) return m3.find_cover(pos);
	else assert(false);
	return std::pair<PosType, PosType>();
}

PNIntv::PosType PNIntv::rank_interval(PosType pos) const {
	if (method == 1) return m1.rank_interval(pos);
	else if (method == 2) return m2.rank_interval(pos);
	else if (method == 3) return m3.rank_interval(pos); 
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::find_rlen(PosType val) const {
	if (method == 1) return m1.find_rlen(val);
	else if (method == 2) return m2.find_rlen(val);
	else if (method == 2) return m3.find_rlen(val);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::coverage(PosType pos) const {
	if (method == 1) return m1.coverage(pos);
	else if (method == 2) return m2.coverage(pos);
	else if (method == 3) return m3.coverage(pos);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_start(PosType i) const {
	if (method == 1) return m1.int_start(i);
	else if (method == 2) return m2.int_start(i);
	else if (method == 3) return m3.int_start(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_len(PosType i) const {
	if (method == 1) return m1.int_len(i);
	else if (method == 2) return m2.int_len(i);
	else if (method == 3) return m3.int_len(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_end(PosType i) const {
	if (method == 1) return m1.int_end(i);
	else if (method == 2) return m2.int_end(i);
	else if (method == 3) return m3.int_end(i);
	else assert(false);
	return 0;
}

PNIntv::PosType PNIntv::int_psrlen(PosType i) const {
	if (method == 1) return m1.int_psrlen(i);
	else if (method == 2) return m2.int_psrlen(i);
	else if (method == 3) return m3.int_psrlen(i);
	else assert(false);
	return 0;
}

std::pair<PNIntv::PosType, PNIntv::PosType> PNIntv::int_startend(PNIntv::PosType i) const {
	if (method == 1) return m1.int_startend(i);
	else if (method == 2) return m2.int_startend(i);
	else if (method == 3) return m3.int_startend(i);
	else {
		assert(false);
		return std::pair<PNIntv::PosType, PNIntv::PosType>();
	}
}

PNIntv::PosType PNIntv::length() const {
	if (method == 1) return m1.length();
	else if (method == 2) return m2.length();
	else if (method == 3) return m3.length();
	else assert(false);
	return 0;
}

void PNIntv::getEnum( PosType idx, Enum *e ) const {
	e->method = method;
	if (method == 1) m1.getEnum(idx, &(e->e1));
	else if (method == 2) m2.getEnum(idx, &(e->e2));
	else if (method == 3) m3.getEnum(idx, &(e->e3));
	else throw std::runtime_error("not initilized");
}

void PNIntv::inspect( const std::string& cmd, std::ostream& out ) const {
	if (method == 1) m1.inspect(cmd, out);
	else if (method == 2) m2.inspect(cmd, out);
	else if (method == 3) m3.inspect(cmd, out);
}



} // namespace
