
#include "rank_vals.h"
#include <set>
namespace app_ds {


void RankValArrBuilder::init(unsigned int _method, unsigned int rate) {
	rbd.init(_method, rate);
}

void RankValArrBuilder::build(RankValArr* out) {
	std::set<unsigned int> dist;
	for (auto it = vals.begin(); it != vals.end(); ++it) 
		dist.insert(*it);
	for (auto it = dist.begin(); it != dist.end(); ++it) 
		vbd.add_inc(*it);
	vbd.build(&(out->vals));
	for (auto it = vals.begin(); it != vals.end(); ++it) 
		rbd.add(out->vals.rank(*it));
	rbd.build(&(out->rankv));
	vals.clear();
}

void RankValArrBuilder::build(mscds::OArchive& ar) {
	RankValArr out;
	build(&out);
	out.save(ar);
}


void RankValArr::save(mscds::OArchive& ar) const {
	ar.startclass("rank_array_values", 1);
	rankv.save(ar.var("rank_values"));
	vals.save(ar.var("value_list"));
	ar.endclass();
}

void RankValArr::load(mscds::IArchive& ar) {
	ar.loadclass("rank_array_values");
	rankv.load(ar.var("rank_values"));
	vals.load(ar.var("value_list"));
	ar.endclass();
}

void RankValArr::getEnum(size_t idx, Enum * e) const {
	e->ptr = &(this->vals);
	rankv.getEnum(idx, &(e->e));
}

uint64_t RankValArr::access( size_t p ) {
	return vals.prefixsum(rankv.access(p));
}


bool RankValArr::Enum::hasNext() const {
	return e.hasNext();
}

uint64_t RankValArr::Enum::next() {
	return ptr->prefixsum(e.next());
}


}//namespace

