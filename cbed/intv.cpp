#include "intv.h"

namespace app_ds {

void IntvLstBuilder::build(mscds::OArchive &ar) {
	IntvLst out;
	build(&out);
	out.save(ar);
}

void IntvLstBuilder::clear() { len = 0;  cur_id = 0; last_st = 0; last_ed = 0; lst.clear(); }

void IntvLstBuilder::add(IntvLstBuilder::PosType st, IntvLstBuilder::PosType ed) {
	if (ed <= st) throw std::runtime_error("wrong interval");
	if (st < last_st) throw std::runtime_error("invalid order");
	if (st == last_st && ed < last_ed) throw std::runtime_error("invalid order");
	lst.push_back(PosData(cur_id, st, '<'));
	lst.push_back(PosData(cur_id, ed, '>'));
	cur_id++;
}

void IntvLstBuilder::build(IntvLst *out) {
	auto idcmp = [](const PosData& a, const PosData& b)->bool {
		if (a.pos != b.pos) return a.pos < b.pos;
		else return a.id < b.id;
	};
	//sort by id
	std::sort(lst.begin(), lst.end(), idcmp);
	mscds::OBitStream markout;
	mscds::SDArraySmlBuilder bd;
	//scan
	size_t i = 0;
	for (auto it = lst.begin(); it != lst.end(); ++it) {
		if (it->type == '<') markout.put1();
		else markout.put0();
		bd.add_inc(it->pos);
		it->pos_order = i;
		++i;
	}
	bd.build(&(out->pos));
	markout.close();
	mscds::Rank6pBuilder rbd;
	rbd.build(mscds::BitArray::create(markout.data_ptr(), markout.length()), &(out->marks));
	///
	std::sort(lst.begin(), lst.end(), [](const PosData& a, const PosData& b)->bool{
		if (a.id != b.id) return a.id < b.id;
		else return '<' == a.type;
	});

	auto it = lst.begin();
	while (it != lst.end()) {
		auto jt = it;
		++jt;
		assert(jt != lst.end() && it->id == jt->id && it->type == '<' && jt->type == '>');
		it->span = jt->pos_order - it->pos_order;
		jt->span = it->span;
		++jt;
		it = jt;
	}
	//
	mscds::SDArraySmlBuilder bdsp;
	std::sort(lst.begin(), lst.end(), idcmp);
	unsigned int stcnt = 0;
	for (auto it = lst.begin(); it != lst.end(); ++it) {
		bdsp.add(it->span);
		if (it->type == '<') {
			assert(it->id == stcnt);
			stcnt++;
		}
	}
	bdsp.build(&(out->span));
}

void IntvLst::save(mscds::OArchive &ar) const {
	ar.startclass("Interval_list");
	marks.save(ar.var("markers"));
	pos.save(ar.var("positions"));
	span.save(ar.var("span"));
	ar.endclass();
}

void IntvLst::load(mscds::IArchive &ar) {
	ar.loadclass("Interval_list");
	marks.load(ar.var("markers"));
	pos.load(ar.var("positions"));
	span.load(ar.var("span"));
	ar.endclass();
}

void IntvLst::clear() { marks.clear(); pos.clear(); span.clear(); }

std::pair<IntvLst::PosType, IntvLst::PosType> IntvLst::get(unsigned int i) const {
	std::pair<PosType, PosType> ret;
	auto rpos = marks.select(i);
	ret.first = (PosType) pos.prefixsum(i);
	auto span = pos.lookup(i);
	ret.second = (PosType) pos.prefixsum(i + span);
	return ret;
}


}//namespace
