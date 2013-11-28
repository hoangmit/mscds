#pragma once

#include "nintv.h"
#include "valrange.h"
#include <vector>
#include <deque>



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

inline std::deque<app_ds::ValRange> genInp(const std::vector<int>& A) {
	std::deque<app_ds::ValRange> inp;
	int len = A.size();
	int lastv = 0, start;
	for (int i = 0; i < len; i++) {
		if (A[i] != lastv) {
			if (lastv != 0)
				inp.push_back(app_ds::ValRange(start, i, lastv));
			start = i;
		}
		lastv = A[i];
	}
	if (lastv != 0)
		inp.push_back(app_ds::ValRange(start, len, lastv));
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