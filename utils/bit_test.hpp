#pragma once

#include <sstream>
#include <utility>
#include <stdint.h>

namespace utils {
	/** \brief returns 64 bits in binary string of input "n" for testing purpose */
	inline std::string binstr(uint64_t n) {
		std::ostringstream ss;
		for (int i = 0; i < 64; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
			if (n == 0) break;
		}
		return ss.str();
	}
	/** \brief returns "len" bits in binary string of input "n" for testing purpose */
	inline std::string binstr(uint64_t n, int len) {
		std::ostringstream ss;
		for (int i = 0; i < len; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
		}
		return ss.str();
	}

	/** \brief same as binstr(uint64_t n, int len) but in a different input format */
	template<typename T>
	inline std::string binstr(const std::pair<uint64_t, T>& p) {
		std::ostringstream ss;
		uint64_t n = p.first;
		for (int i = 0; i < p.second; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
		}
		return ss.str();
	}

	#define WATCH(x) cout<< #x <<"="<<(x)<<endl;
}
