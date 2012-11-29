#pragma once

#ifndef __COUNT_2D_H_
#define __COUNT_2D_H_

#include <stdint.h>
#include "archive.h"
#include "wat_array.h"
#include "bitarray/sdarray.h"

namespace mscds{

struct Point {
	Point() {}
	Point(const Point& p): x(p.x), y(p.y) {}
	Point(unsigned int _x, unsigned int _y): x(_x), y(_y) {}
	unsigned int x, y;
};


class Count2DBuilder {
public:
	void build(const std::vector<Point>& list, Count2D * out);
	void build(const std::vector<Point>& list, OArchive& ar);
};

class Count2D {
public:
	uint64_t count(unsigned int x, unsigned int y) const;
	std::vector<unsigned int> count_grid(const std::vector<unsigned int>& X, const std::vector<unsigned int>& Y) const;
	void load(IArchive& ar);
private:
	WatQuery wq;
	SDArrayQuery SX, SY, DPX;
	unsigned int max_x, max_y;
};

} //namespace

#endif //__COUNT_2D_H_