#pragma once

#include <stdint.h>
#include <string>
#include <functional>

namespace utils {

/* Fowler / Noll / Vo (FNV) Hash */
struct FNV_hash {

	/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
	static const uint32_t InitialFNV = 2166136261U;
	static const uint32_t FNVMultiple = 16777619;

	static uint32_t hash32(const std::string& s) {
		uint32_t hash = InitialFNV;
		for (size_t i = 0; i < s.length(); i++) {
			hash = hash ^ (s[i]);       /* xor the low 8 bits */
			hash = hash * FNVMultiple;  /* multiply by the magic number */
		}
		return hash;
	}

	static uint32_t hash24(const std::string& s) {
		uint32_t hash = hash32(s);
		return (hash >> 24) ^ (hash & 0xFFFFFFU);
	}
};

template<typename T> inline void hash_combine(size_t & seed, T const& v) {
	seed ^= std::hash(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


/* The MIT License

Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)
...
*/


uint32_t fasthash32(const void *buf, size_t len, uint32_t seed=3);

uint64_t fasthash64(const void *buf, size_t len, uint64_t seed=3);

inline uint32_t fasthash32(const std::string& s, uint32_t seed = 3) {
	return fasthash32(s.c_str(), s.length(), seed);
}

inline uint64_t fasthash64(const std::string& s, uint64_t seed = 3) {
	return fasthash64(s.c_str(), s.length(), seed);
}

}
