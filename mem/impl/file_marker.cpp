#include "file_marker.h"

namespace mscds {

/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
static const uint32_t InitialFNV = 2166136261U;
static const uint32_t FNVMultiple = 16777619;

/* Fowler / Noll / Vo (FNV) Hash */
uint32_t FNV_hash32(const std::string& s) {
	uint32_t hash = InitialFNV;
	for (size_t i = 0; i < s.length(); i++) {
		hash = hash ^ (s[i]);       /* xor the low 8 bits */
		hash = hash * FNVMultiple;  /* multiply by the magic number */
	}
	return hash;
}

uint32_t FNV_hash24(const std::string& s) {
	uint32_t hash = FNV_hash32(s);
	return (hash >> 24) ^ (hash & 0xFFFFFFU);
}

}
