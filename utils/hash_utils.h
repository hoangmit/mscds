#pragma once

#include <stdint.h>
#include <string>

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


/* The MIT License

Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
