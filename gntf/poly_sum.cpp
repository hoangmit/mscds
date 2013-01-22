#include "poly_sum.h"

namespace app_ds {

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



}//namespace