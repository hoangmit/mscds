#pragma once

#ifndef __COUNT_2D_H_
#define __COUNT_2D_H_

#include <stdint.h>
#include "framework/archive.h"
#include "wat_array.h"
#include "intarray/sdarray_sml.h"

namespace mscds{

struct Point {
	Point() {}
	Point(const Point& p): x(p.x), y(p.y) {}
	Point(unsigned int _x, unsigned int _y): x(_x), y(_y) {}
	unsigned int x, y;

	bool operator< (const Point& p) const {
		if (x != p.x) return x < p.x;
		else return y < p.y;
	}
};

class Count2DQuery;

class Count2DBuilder {
public:
	void build(std::vector<Point>& list, Count2DQuery * out);
	void build(std::vector<Point>& list, OutArchive& ar);
};

class Count2DQuery {
public:
	uint64_t count(unsigned int x, unsigned int y) const;
	std::vector<unsigned int> count_grid(const std::vector<unsigned int>& X, const std::vector<unsigned int>& Y) const;
	std::vector<unsigned int> heatmap(unsigned int x1, unsigned int x2, 
		unsigned int y1, unsigned int y2, unsigned int nx, unsigned int ny) const;
	typedef WatQuery::ListCallback ListCallback;
	/** return the points in  */
	void list_each(uint64_t min_x, uint64_t max_x, uint64_t min_y, uint64_t max_y, ListCallback cb, void* context) const;

	void clear();
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	size_t size();
private:
	WatQuery wq;
	SDRankSelectSml SX, SY, DPX;
	unsigned int max_x, max_y;
	friend class Count2DBuilder;

	unsigned int map_x(unsigned int x) const;
	unsigned int map_y(unsigned int y) const;
	unsigned int unmap_x(unsigned int x) const;
	unsigned int unmap_y(unsigned int y) const;

};

} //namespace

#endif //__COUNT_2D_H_
