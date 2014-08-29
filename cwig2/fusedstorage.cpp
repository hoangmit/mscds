#include "fusedstorage.h"

#include "utils/prec_summation.h" // not used yet

namespace app_ds {

Storage::Storage(): itv(data.g<0>()), vals(data.g<1>()), sumq(data.g<2>()),
	sqrsum(data.g<3>()) {}

void Storage::clear() {data.clear(); fmap.clear();}

void Storage::load(mscds::InpArchive &ar) {
	ar.loadclass("storage1");
	data.load(ar.var("data"));
	fmap.load(ar.var("fmap"));
	ar.endclass();
}

void Storage::save(mscds::OutArchive &ar) const {
	ar.startclass("storage1");
	data.save(ar.var("data"));
	fmap.save(ar.var("fmap"));
	ar.endclass();
}

double Storage::get_val(unsigned int idx) const {
	return fmap.unmap_sf(vals.get(idx));
}

double Storage::get_sumq(unsigned int idx) const {
	return fmap.unmap_if(sumq.get(idx));
}

double Storage::get_sqrsum(unsigned int idx) const {
	return fmap.unmap_if(sqrsum.get(idx));
}

size_t Storage::length() const {
	return itv.length();
}

void Storage::getEnum(size_t base, Storage::Enum *e) const {
	vals.getEnum(base, &(e->e));
	e->self = this;
}

bool Storage::Enum::hasNext() const { return e.hasNext(); }

double Storage::Enum::next() {
	return self->fmap.unmap_sf(e.next());
}

void StorageBuilder::add(unsigned int st, unsigned int ed, double val) {
	data.emplace_back(st, ed, val);
}

void StorageBuilder::build(Storage *qs) {
	std::deque<ValRange>::const_iterator it;
	BD base;
	auto& posbd = base.g<0>();
	auto& valbd = base.g<1>();
	auto& sumbd = base.g<2>();
	auto& sqsumbd = base.g<3>();

	for (it = data.cbegin(); it != data.cend(); ++it)
		rvbd.add(it->val);
	rvbd.build(&qs->fmap);
	posbd.start_model();
	valbd.start_model();
	for (it = data.cbegin(); it != data.cend(); ++it) {
		uint64_t valt = qs->fmap.map_fs(it->val);
		valbd.model_add(valt);
		posbd.model_add(it->st);
	}
	valbd.build_model();
	posbd.build_model();
	sumbd.init_blk(8);
	sqsumbd.init_blk(8);
	base.init();
	double psum = 0;
	double sqpsum = 0;
	unsigned int lastst = 0;

	size_t i = 0;

	for (it = data.cbegin(); it != data.cend(); ++it) {
		if (i % SUM_GAP == 0) {
			sumbd.add(qs->fmap.map_fi(psum));
			sqsumbd.add(qs->fmap.map_fi(sqpsum));
		}
		posbd.add(it->st, it->ed);
		int64_t vali = qs->fmap.map_fi(it->val);
		valbd.add(qs->fmap.map_is(vali));
		if (it->st < lastst) throw std::runtime_error("overlapping intervals");
		unsigned int llen = it->ed - it->st;
		psum += llen * it->val;
		sqpsum += llen * (it->val*it->val);
		lastst = it->st;
		++i;
		base.check_end_block();
		//if (i % 512 == 0) { base._end_block(); }
	}
	base.check_end_data();
	//if (i % 512 != 0) base._end_block();
	base.build(&qs->data);
}

void StorageBuilder::build(mscds::OutArchive &ar) {
	Storage qs;
	build(&qs);
	qs.save(ar);
}

}//namespace
