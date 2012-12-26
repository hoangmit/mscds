#include "count2d.h"


#include <vector>
#include <algorithm>



namespace mscds{
	
using namespace std;

void Count2DBuilder::build(std::vector<Point>& list, Count2DQuery * out) {
	assert(list.size() < (1ULL<<32));
	assert(list.size() > 0);
	out->clear();
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
	out->SY.build(Y);

	SDArraySmlBuilder xbd, dp;
	//work on X
	if (X.size() > 0) {
		vector<unsigned int> Cnt;
		Cnt.push_back(0);
		for (unsigned int i = 1; i < X.size(); ++i)
			if (X[i] != X[i-1]) Cnt.push_back(i);
		Cnt.push_back(X.size());
		out->DPX.build(Cnt);
	}
	X.erase(unique(X.begin(), X.end()), X.end());
	out->SX.build(X);
	
	// work on points
	vector<uint64_t> wlst;
	sort(list.begin(), list.end());
	for (unsigned int i = 0; i < list.size(); ++i)
		wlst.push_back(out->SY.rank(list[i].y));
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
	DPX.save(ar);
	wq.save(ar);
	ar.endclass();
}


void Count2DQuery::load(IArchive& ar) {
	clear();
	ar.loadclass("count2d");
	ar.var("max_x").load(max_x);
	ar.var("max_y").load(max_y);
	SX.load(ar);
	SY.load(ar);
	DPX.load(ar);
	wq.load(ar);
	ar.endclass();
}

void Count2DQuery::clear() {
	max_x = 0;
	max_y = 0;
	wq.clear();
	SX.clear();
	SY.clear();
	DPX.clear();
}

uint64_t Count2DQuery::count(unsigned int x, unsigned int y) const {
	return wq.rankLessThan(map_y(y), map_x(x));
}

unsigned int Count2DQuery::map_x(unsigned int x) const {
	return DPX.select(SX.rank(x));
}

unsigned int Count2DQuery::map_y(unsigned int y) const {
	return SY.rank(y);
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

size_t Count2DQuery::size() {
	return wq.length();
}



}//namespace
