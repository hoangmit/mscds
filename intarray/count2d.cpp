#include "count2d.h"


#include <vector>
#include <algorithm>



namespace mscds{
	
using namespace std;

void Count2DBuilder::build(std::vector<Point>& list, Count2DQuery * out) {
	assert(list.size() > 0);
	vector<unsigned int> X(list.size()), Y(list.size());
	for (unsigned int i = 0; i < list.size(); ++i) {
		X[i] = list[i].x;
		Y[i] = list[i].y;
	}
	std::sort(X.begin(), X.end());
	std::sort(Y.begin(), Y.end());
	out->max_x = X.back();
	out->max_y = Y.back();
	//work on Y
	Y.erase(unique(Y.begin(), Y.end()), Y.end());
	SDArrayBuilder ybd;
	unsigned int last = 0;
	for (unsigned int i = 0; i < Y.size(); ++i) {
		ybd.add(Y[i] - last);
		last = Y[i];
	}
	ybd.build(&(out->SY));

	SDArrayBuilder xbd, dp;
	//work on X
	unsigned int cnt = 1;
	last = X[0];
	xbd.add(X[0] - 0);
	for (unsigned int i = 1; i < X.size(); ++i) {
		if (X[i] != last) {
			xbd.add(X[i] - last);
			dp.add(cnt);
			last = X[i];
			cnt = 1;
		}else
			cnt++;
	}
	dp.add(cnt);
	// work on points
	xbd.build(&out->SX);
	dp.build(&out->DPX);

	vector<uint64_t> wlst;
	last = list[0].x;
	//unsigned int pos = 0;
	sort(list.begin(), list.end());
	for (unsigned int i = 0; i < list.size(); ++i) {
		//if (list[i].x != last) pos++;
		wlst.push_back(out->SY.find(list[i].y)-1);
	}
	WatBuilder bd;
	bd.build(wlst, &out->wq);
}

void Count2DBuilder::build(std::vector<Point>& list, OArchive& ar) {
	Count2DQuery out;
	build(list, &out);
	out.save(ar);
}

void Count2DQuery::save(OArchive& ar) const {
	ar.startclass("count2d", 1);
	ar.var("max_x").save(max_x);
	ar.var("max_y").save(max_y);
	SX.save(ar);
	SY.save(ar);
	wq.save(ar);
	ar.endclass();
}

uint64_t Count2DQuery::count(unsigned int x, unsigned int y) const {
	return wq.rankLessThan(map_y(y), map_x(x));
}

unsigned int Count2DQuery::map_x(unsigned int x) const {
	uint64_t p = 0;
	if (x <= max_x) {
		uint64_t cx = SX.find(x);
		if (cx == 0) return 0;
		if (SX.prefixsum(cx) == x) cx--;
		p = DPX.prefixsum(cx);
	} else {
		p = DPX.prefixsum(DPX.length());
	}
	return p;
}

unsigned int Count2DQuery::map_y(unsigned int y) const {
	uint64_t cy;
	if (y <= max_y) {
		cy = SY.find(y);
		if (cy == 0) return 0;
		if (SY.prefixsum(cy) == y) cy--;
	} else {
		cy = SY.length();
	}
	return cy;
}

std::vector<unsigned int> Count2DQuery::count_grid(const std::vector<unsigned int>& X, const std::vector<unsigned int>& Y) const {
	GridQuery gq;
	std::vector<unsigned int> Xp(X), Yp(Y);
	for (unsigned int i = 0; i < Xp.size(); i++) 
		Xp[i] = map_x(Xp[i]);
	for (unsigned int i = 0; i < Yp.size(); i++) 
		Yp[i] = map_y(Yp[i]);
	sort(Xp.begin(), Xp.end());
	sort(Yp.begin(), Yp.end());
	std::vector<unsigned int> result;
	gq.process(&wq,  Xp, Yp, &result);
	return result;
}



}//namespace
