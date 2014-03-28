#pragma once

#include "nintv.h"
#include <vector>
#include <deque>

struct ValRangeInfo {
	ValRangeInfo() {}
	ValRangeInfo(unsigned _st, unsigned _ed, int _val) : st(_st), ed(_ed), val(_val) {}
	unsigned int st, ed;
	int val;
};


inline std::vector<int> gen_density(int len, unsigned int val_range = 5) {
	int last = 0;
	std::vector<int> ret(len);
	for (int i = 0; i < len; i++) {
		if (last == 0) {
			if (rand() % 100 < 50) ret[i] = 0;
			else ret[i] = (rand() % val_range) + 1;
		}
		else {
			if (rand() % 100 < 90) ret[i] = last;
			else {
				if (rand() % 100 < 40) ret[i] = (rand() % val_range) + 1;
				else ret[i] = 0;
			}
		}
		last = ret[i];
	}
	return ret;
}

inline std::deque<ValRangeInfo> genInp(const std::vector<int>& A) {
	std::deque<ValRangeInfo> inp;
	int len = A.size();
	int lastv = 0, start;
	for (int i = 0; i < len; i++) {
		if (A[i] != lastv) {
			if (lastv != 0)
				inp.emplace_back(start, i, lastv);
			start = i;
		}
		lastv = A[i];
	}
	if (lastv != 0)
		inp.emplace_back(start, len, lastv);
	return inp;
}

inline std::vector<std::pair<unsigned int, unsigned int> > gen_intv(unsigned int len, double consec = 0.5, unsigned int intlen = 5) {
	std::vector<std::pair<unsigned int, unsigned int> > ret;
	ret.reserve(len);
	unsigned int thres = consec * 10000;

	unsigned int lasted = rand() % intlen;
	for (unsigned int i = 0; i < len; ++i) {
		unsigned nlen = rand() % intlen + 1;
		if (rand() % 10000 < thres) {
			ret.emplace_back(lasted, lasted + nlen);
			lasted += nlen;
		} else {
			unsigned int nst = lasted + 1 + rand() % intlen;
			ret.emplace_back(nst, nst + nlen);
			lasted = nst + nlen;
		}
	}
	return ret;
}

inline std::vector<std::pair<unsigned int, unsigned int> > convert2pair(const std::deque<ValRangeInfo>& vec) {
	std::vector<std::pair<unsigned int, unsigned int> > ret;
	ret.reserve(vec.size());
	for (auto& v : vec) {
		ret.emplace_back(v.st, v.ed);
	}
	return ret;
}