#pragma once

/** \file

Common random generator for string, vector and bits

*/

#include <string>
#include <vector>
#include <cstdlib>

namespace tests {

/// generates random string
inline std::string generate_str(size_t len, const std::string& alph = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") {
	std::string out;
	for (size_t i = 0; i < len; ++i) {
		char ch = alph[rand() % alph.size()];
		out.push_back(ch);
	}
	return out;
}

/// generate random vector
template<typename T = unsigned int>
inline std::vector<T> rand_vec(unsigned int len, int range = 1000) {
	std::vector<T> out(len);
	for (unsigned int i = 0; i < len; ++i)
		out[i] = (rand() % range);
	return out;
}

/// generate random bit vector
inline std::vector<bool> rand_bitvec(unsigned int len, double true_percentage = 50) {
	// this implementation is only accurate up to 0.01%
	std::vector<bool> bv(len);
	unsigned int cutoff = (unsigned int)(true_percentage * 100);
	for (unsigned int i = 0; i < len; ++i) {
		bv[i] = (rand() % 10000) < cutoff;
	}
	return bv;
}

}//namespace
