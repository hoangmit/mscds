#include "poly_sum.h"

#include <stdexcept>

namespace app_ds {

PRSumArrBuilder::PRSumArrBuilder() {
	init(0, 32);
}

void PRSumArrBuilder::init(unsigned int method, unsigned int rate) {
	this->rate = rate;
	this->method = method;
	cnt = 0;
	lastval = 0;
}

void PRSumArrBuilder::add(unsigned int v) {
	if (method == 0 && cnt <= CHECK_THRESHOLD) {
		vals.push_back(v);
		if (cnt == CHECK_THRESHOLD)
			choosemethod();
	} else {
		addmethod(v);
	}
	cnt++;
}

void PRSumArrBuilder::build(PRSumArr* out) {
	if (method == 0 && cnt <= CHECK_THRESHOLD)
		choosemethod();
	out->len = cnt;
	out->storetype = method;
	out->rate = rate;
	if (method == 1) sdab.build(&(out->sda));
	else if(method == 2) dt1.build(&(out->dt1));
	else if(method == 3) dt2.build(&(out->dt2));
}

void PRSumArrBuilder::choosemethod() {
	sdab.clear();
	dt1.clear();
	dt2.clear();
	for (size_t i = 0; i < vals.size(); ++i) {
		unsigned int vx = vals[i];
		sdab.add(vx);
		dt1.add(vx);
		dt2.add(vx);
	}
	mscds::OClassInfoArchive ar;
	sdab.build(ar);
	size_t s1 = ar.opos();
	dt1.build(ar);
	size_t s2 = ar.opos() - s1;
	dt2.build(ar);
	size_t s3 = ar.opos() - s1 - s2;
	ar.close();
	size_t sx = s1;
	method = 1;
	if (s1 > s2) { method = 2; sx = s2; }
	if (s2 > s3) { method = 3; sx = s3; }
	resetbd();
	for (size_t i = 0; i < vals.size(); ++i)
		addmethod(vals[i]);
	vals.clear();
}

void PRSumArrBuilder::resetbd() {
	sdab.clear();
	lastval = 0;
	dt1.init(rate);
	dt2.init(rate);
}

void PRSumArrBuilder::addmethod(unsigned int val) {
	if (method==1) sdab.add(val);
	else if (method==2) dt1.add(val);
	else if (method ==3) dt2.add(val);
	else assert(false);
}

mscds::EnumeratorInt<uint64_t>* PRSumArr::getEnum(size_t idx) const  {
	if (storetype==1) {
		return new mscds::SDArraySml::Enum(sda.getEnum(idx));
	} else if (storetype==2) {
		return new mscds::DeltaCodeArr::Enumerator(dt1.getEnum(idx));
	} else if (storetype ==3) {
		return new mscds::DiffDeltaArr::Enumerator(dt2.getEnum(idx));
	} else { throw std::runtime_error("wrong type");
		return NULL;
	}
}

uint64_t PRSumArr::access(size_t p) {
	return 0;
}

void PRSumArr::save(mscds::OArchive& ar) const {
	ar.startclass("polymorphic_array", 1);
	ar.var("length").save(len);
	ar.var("method").save(storetype);
	ar.var("rate").save(rate);
	sda.save(ar.var("sda"));
	dt1.save(ar.var("delta"));
	dt2.save(ar.var("diff_delta"));
	ar.endclass();
}

void PRSumArr::load(mscds::IArchive& ar) {
	ar.loadclass("polymorphic_array");
	ar.var("length").load(len);
	ar.var("method").load(storetype);
	ar.var("rate").load(rate);
	sda.load(ar.var("sda"));
	dt1.load(ar.var("delta"));
	dt2.load(ar.var("diff_delta"));
	ar.endclass();
	if (storetype == 0) throw std::runtime_error("unknown type");
}


void PRSumArr::clear() {
	len = 0;
	rate = 0;
	storetype = 0;
	sda.clear();
	dt1.clear();
	dt2.clear();
}


}//namespace