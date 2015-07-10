#pragma once

#include <stdint.h>
#include <string>
#include <functional>

/**  \file

Collection of hash functions

*/

namespace utils {

/** Fowler / Noll / Vo (FNV) Hash */
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
	seed ^= std::hash<T>(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/// Pearson hash
class PearsonHash {
public:
	template<typename T = uint32_t>
	static T hash(const void *buf, size_t len) {
		T ret;
		_pearson_hash_mix(buf, len, &ret, sizeof(ret));
		return ret;
	}

	template<typename T = uint32_t>
	static T hash(const std::string& s) {
		return hash<T>(s.c_str(), s.length());
	}

private:
	static void _pearson_hash(const void *buf, size_t len, void* out, size_t osize);
};

/* The MIT License

Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)
...
*/

class FastHash {
public:
	static uint32_t hash32(const void *buf, size_t len, uint32_t seed = 3);

	static uint64_t hash64(const void *buf, size_t len, uint64_t seed = 3);

	static uint32_t hash32(const std::string& s, uint32_t seed = 3) {
		return hash32(s.c_str(), s.length(), seed);
	}

	static uint64_t fasthash64(const std::string& s, uint64_t seed = 3) {
		return hash64(s.c_str(), s.length(), seed);
	}
};

}
