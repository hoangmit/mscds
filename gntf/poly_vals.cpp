#include "poly_vals.h"
#include "mem/filearchive.h"
#include <stdexcept>
#include "utils/param.h"

namespace app_ds {

PRValArrBuilder::PRValArrBuilder() {
	init(0, 32);
}

void PRValArrBuilder::init(unsigned int _method, unsigned int rate) {
	this->rate = rate;
	this->method = _method;
	autoselect = (method == 0);
	dt1.init(rate);
	dt2.init(rate);
	cnt = 0;
	lastval = 0;
	vals.clear();
}


void PRValArrBuilder::clear() {
	cnt = 0; lastval = 0; vals.clear(); sdab.clear(); dt1.clear(); dt2.clear();
	if (autoselect) method = 0; 
}

void PRValArrBuilder::add(unsigned int v) {
	if (method == 0 && cnt <= CHECK_THRESHOLD) {
		vals.push_back(v);
		if (cnt == CHECK_THRESHOLD)
			choosemethod();
	} else {
		addmethod(v);
	}
	cnt++;
}

void PRValArrBuilder::build(PRValArr* out) {
	if (method == 0 && cnt <= CHECK_THRESHOLD)
		choosemethod();
	out->len = cnt;
	out->storetype = method;
	out->rate = rate;
	out->autoselect = (int) autoselect;
	if (method == 1) sdab.build(&(out->sda));
	else if(method == 2) dt1.build(&(out->dt1));
	else if(method == 3) dt2.build(&(out->dt2));
}

void PRValArrBuilder::choosemethod() {
	sdab.clear();
	dt1.clear();
	dt2.clear();
	for (size_t i = 0; i < vals.size(); ++i) {
		unsigned int vx = vals[i];
		sdab.add(vx);
		dt1.add(vx);
		dt2.add(vx);
	}
	mscds::OSizeEstArchive ar;
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

void PRValArrBuilder::resetbd() {
	sdab.clear();
	lastval = 0;
	dt1.init(rate);
	dt2.init(rate);
}

void PRValArrBuilder::addmethod(unsigned int val) {
	if (method==1) sdab.add(val);
	else if (method==2) dt1.add(val);
	else if (method ==3) dt2.add(val);
	else assert(false);
}

void PRValArr::Enum::init(int _etype) {
	this->etype = _etype;
	if (etype == 1) {
		if (e1 == NULL) e1 = new mscds::SDArraySml::Enum();
	} else if (etype == 2) {
		if (e2 == NULL) e2 = new mscds::DeltaCodeArr::Enum();
	} else if (etype == 3) {
		if (e3 == NULL) e3 = new mscds::DiffDeltaArr::Enum();
	}
}

PRValArr::Enum::~Enum() {
	if (e1 != NULL) delete e1;
	if (e2 != NULL) delete e2;
	if (e3 != NULL) delete e3;
}

bool PRValArr::Enum::hasNext() const {
	switch (etype) {
	case 1: return e1->hasNext();
	case 2: return e2->hasNext();
	case 3: return e3->hasNext();
	}
	return false;
}

uint64_t PRValArr::Enum::next() {
	switch (etype) {
	case 1: return e1->next();
	case 2: return e2->next();
	case 3: return e3->next();
	}
	return 0;
}

void PRValArr::getEnum(size_t idx, Enum * e) const  {
	e->init(storetype);
	if (storetype==1) {
		sda.getEnum(idx, (e->e1));
	} else if (storetype==2) {
		dt1.getEnum(idx, (e->e2));
	} else if (storetype ==3) {
		dt2.getEnum(idx, e->e3);
	} else { throw std::runtime_error("wrong type");
	}
}

uint64_t PRValArr::access(size_t p) const {
	if (storetype == 1) return sda.lookup(p);
	else 
	if (storetype == 2) {
		mscds::DeltaCodeArr::Enum e;
		dt1.getEnum(p, &e);
		return e.next();
	}
	else
	if (storetype==3) {
		mscds::DiffDeltaArr::Enum e;
		dt2.getEnum(p, &e);
		return e.next();
	}
	else {
		throw std::runtime_error("unknow type");
		return 0;
	}
}

void PRValArr::save(mscds::OArchive& ar) const {
	ar.startclass("polymorphic_array", 1);
	ar.var("length").save(len);
	ar.var("method").save(storetype);
	ar.var("rate").save(rate);
	ar.var("autoselect").save(autoselect);
	if (storetype == 1) sda.save(ar.var("sda"));
	if (storetype == 2) dt1.save(ar.var("delta"));
	if (storetype == 3) dt2.save(ar.var("diff_delta"));
	ar.endclass();
}

void PRValArr::load(mscds::IArchive& ar) {
	ar.loadclass("polymorphic_array");
	ar.var("length").load(len);
	ar.var("method").load(storetype);
	ar.var("rate").load(rate);
	ar.var("autoselect").load(autoselect);
	if (storetype == 1) sda.load(ar.var("sda"));
	if (storetype == 2) dt1.load(ar.var("delta"));
	if (storetype == 3) dt2.load(ar.var("diff_delta"));
	ar.endclass();
	if (storetype == 0) throw std::runtime_error("unknown type");
}

void PRValArr::clear() {
	len = 0;
	rate = 0;
	storetype = 0;
	sda.clear();
	dt1.clear();
	dt2.clear();
}




}//namespace
