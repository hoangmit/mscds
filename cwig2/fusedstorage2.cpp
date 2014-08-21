#include "fusedstorage2.h"

#include "utils/prec_summation.h" // not used yet


/** 
TODO: use a more floating-point stable arithmetic routine. E.g. Those in Boost.Accumulators
Because the floating point errors can be accumulated after summation and muliplication.
*/

namespace app_ds {

Storage2::Storage2(): itv(data.g<0>()), vals(data.g<1>()), sumq(data.g<2>()),
	sqrsum(data.g<3>()) {}

void Storage2::clear() {data.clear(); fmap.clear();}

void Storage2::load(mscds::InpArchive &ar) {
	ar.loadclass("storage2");
	data.load(ar.var("data"));
	fmap.load(ar.var("fmap"));
	ar.endclass();
}

void Storage2::save(mscds::OutArchive &ar) const {
	ar.startclass("storage2");
	data.save(ar.var("data"));
	fmap.save(ar.var("fmap"));
	ar.endclass();
}

double Storage2::get_val(unsigned int idx) const {
	unsigned int r = idx % 64;
	unsigned int p = idx - r;
	mscds::HuffBlkQuery::Enum e;
	vals.getEnum(p, &e);
	double last = fmap.unmap_sf(e.next());
	for (unsigned int i = 0; i < r; ++i) {
		double v = fmap.unmap_sf(e.next());
		last += v;
	}
	return last;
}

void Storage2::getEnum(size_t base, Storage2::Enum *e) const {
	unsigned int r = base % 64;
	unsigned int p = base - r;
	vals.getEnum(p, &(e->e));
	e->self = this;
	e->dc.i = p;
	for (unsigned int i = 0; i < r; ++i) { assert(e->hasNext()); e->next(); }
}

double Storage2::get_sumq(unsigned int idx) const {
	return fmap.unmap_if(sumq.get(idx));
}

double Storage2::get_sqrsum(unsigned int idx) const {
	return fmap.unmap_if(sqrsum.get(idx));
}

size_t Storage2::length() const {
	return itv.length();
}

bool Storage2::Enum::hasNext() const { return e.hasNext(); }

double Storage2::Enum::next() {
	return dc.decode(self->fmap.unmap_sf(e.next()));
}

void StorageBuilder2::add(unsigned int st, unsigned int ed, double val) {
	data.emplace_back(st, ed, val);
}

void StorageBuilder2::build(Storage2 *qs) {
	std::deque<ValRange>::const_iterator it;
	BD base;
	auto& posbd = base.g<0>();
	auto& valbd = base.g<1>();
	auto& sumbd = base.g<2>();
	auto& sqsumbd = base.g<3>();
	DiffCoder dc;
	for (it = data.cbegin(); it != data.cend(); ++it) {
		fmapbd.add(dc.encode(it->val));
	}
	fmapbd.build(&qs->fmap);
	
	valbd.start_model();
	dc.reset();
	for (it = data.cbegin(); it != data.cend(); ++it) {
		double valf = dc.encode(it->val);
		uint64_t valt = qs->fmap.map_fs(valf);
		valbd.model_add(valt);
	}
	valbd.build_model();
	sumbd.init_blk(8);
	sqsumbd.init_blk(8);
	base.init();
	double psum = 0;
	double sqpsum = 0;
	unsigned int lastst = 0;

	size_t i = 0;
	dc.reset();
	for (it = data.cbegin(); it != data.cend(); ++it) {
		if (i % SUM_GAP == 0) {
			sumbd.add(qs->fmap.map_fi(psum));
			sqsumbd.add(qs->fmap.map_fi(sqpsum));
		}
		posbd.add(it->st, it->ed);
		double valf = dc.encode(it->val);
		valbd.add(qs->fmap.map_fs(valf));
		if (it->st < lastst) throw std::runtime_error("overlapping intervals");
		unsigned int llen = it->ed - it->st;
		psum += llen * it->val;
		sqpsum += llen * (it->val*it->val);
		lastst = it->st;
		++i;
		base.check_end_block();
	}
	base.check_end_data();
	base.build(&qs->data);
}

void StorageBuilder2::build(mscds::OutArchive &ar) {
	Storage2 qs;
	build(&qs);
	qs.save(ar);
}

double DiffCoder::encode(double d) {
	double v;
	if (i % 64 == 0) 
		v = d;
	else
		v = d - last;
	last = d;
	++i;
	return v;
}

double DiffCoder::decode(double d) {
	double v;
	if (i % 64 == 0)
		v = d;
	else
		v = d + last;
	++i;
	last = v;
	return v;
}

}//namespace
