#pragma once

#ifndef __VALRANGE_H_
#define __VALRANGE_H_

namespace app_ds {

template<typename T>
struct ValRangeG {
	unsigned int st, ed;
	T val;
	std::string annotation;
	ValRangeG() {}
	ValRangeG(unsigned int s, unsigned int e, T v):st(s), ed(e), val(v) {}
	ValRangeG(unsigned int s, unsigned int e, T v, const std::string& ann):st(s), ed(e), val(v), annotation(ann) {}
	bool operator<(const ValRangeG<T>& e) const { return st < e.st; }
	bool operator==(const ValRangeG<T>& e) const {
		return st == e.st && ed == e.ed && val == e.val;
	}
};

typedef ValRangeG<double> ValRange;

}//namespace

#endif //__VALRANGE_H_