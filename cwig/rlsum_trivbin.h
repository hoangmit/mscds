#pragma once

#include <iostream>
#include <deque>
#include <string>
#include <cassert>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <stdint.h>
#include <stdexcept>
#include <cstring>

#include "valrange.h"


class TrivBin {
public:
	typedef double SumETp;
	typedef uint32_t StartETp;
	typedef uint16_t LenETp;
	typedef app_ds::ValRange ValRange;
public:
	size_t len;
	SumETp * sum_;
	StartETp * start;
	LenETp * rlen;
public:
	TrivBin(): len(0) {}
	~TrivBin() { destroy();}

	size_t memory_size() {
		return sizeof(len) + len*(sizeof(SumETp) + sizeof(StartETp) + sizeof(LenETp));
	}

	void build(const std::deque<ValRange>& inp) {
		len = inp.size();
		sum_ = new SumETp[len + 1];
		start = new StartETp[len];
		rlen = new LenETp[len];
		uint32_t i = 0;
		SumETp psum = 0;
		for (auto it = inp.begin(); it != inp.end(); ++it) {
			start[i] = it->st;
			int llen = it->ed - it->st;
			if (llen > std::numeric_limits<uint16_t>::max())
				throw std::runtime_error("too long");
			rlen[i] = llen;
			sum_[i] = psum;
			psum += rlen[i] * it->val;
			i++;
		}
		sum_[i] = psum;
	}

	SumETp sum(uint32_t pos) {
		uint32_t p = std::lower_bound(start, start + len, pos) - start;
		if (p == 0) return 0;
		if (p < len && start[p] == pos) 
			return sum_[p];
		p--;
		assert(start[p] < pos);
		uint32_t kl = pos - start[p];
		if (rlen[p] >= kl) 
			return sum_[p] + kl*((sum_[p+1] - sum_[p])/rlen[p]);
		else
			return sum_[p+1];
	}

	size_t save(std::ostream& fo) {
		fo.write("trb3", 4);
		fo.write((const char*)&len, sizeof(len));		
		if (len > 0) {
			fo.write((const char*)sum_, sizeof(SumETp)*(len+1));
			fo.write((const char*)start, sizeof(StartETp)*(len));
			fo.write((const char*)rlen, sizeof(LenETp)*(len));
		}
		if (!fo) throw std::runtime_error("write error");
		fo.put(0);
		fo.put(0);
		return 4 + sizeof(len) +  len*(sizeof(SumETp) +  sizeof(StartETp) + 
			sizeof(LenETp)) + 2 + (len > 0 ? sizeof(SumETp): 0);
	}

	size_t load(std::istream& fi) {
		destroy();
		char buf[5];
		fi.read(buf, 4);
		buf[4] = 0;
		if (strcmp(buf, "trb3") != 0)
			throw std::runtime_error("wrong magic bytes");
		fi.read((char*) &len, sizeof(len));
		if (len > 0) {
			sum_ = new SumETp[len + 1];
			start = new StartETp[len];
			rlen = new LenETp[len];
			fi.read((char*)sum_, sizeof(SumETp)*(len+1));
			fi.read((char*)start, sizeof(StartETp)*(len));
			fi.read((char*)rlen, sizeof(LenETp)*(len));
		}
		if (!fi) throw std::runtime_error("read error");
		fi.get();
		fi.get();
		return 4 + sizeof(len) +  len*(sizeof(SumETp) +  sizeof(StartETp) + 
			sizeof(LenETp)) + 2 + (len > 0 ? sizeof(SumETp): 0);
	}

	void destroy() {
		if (len > 0) {
			delete[] sum_;
			delete[] start;
			delete[] rlen;
			len = 0;
		}
	}

	void print_stat() {
		if (len > 0) {
			std::cout << " len: " << len << " sum: " << sum_[len];
			std::cout << " max_start: " << start[len - 1];
			std::cout << " max_rlen: " << *std::max_element(rlen, rlen + len);
			std::cout << std::endl;
		}
	}
};
