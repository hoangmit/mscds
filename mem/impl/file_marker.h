#pragma once

#include "framework/archive.h"

#include <string>
#include <stdint.h>

namespace mscds {

class FileMaker {
public:

	struct HeaderBlock {
		uint32_t magic;
		uint32_t reserve;
		uint64_t control_ptr;
	};

	//return the position of the control pointer
	static void file_header(HeaderBlock& fh);
	static void check_file_header(HeaderBlock& fh, size_t& data, size_t& control);

	//-----------------

	static void class_start(OutArchive& out, const std::string& name, unsigned char version);
	static void class_end(OutArchive& out);

	static unsigned char check_class_start(InpArchive& inp, const std::string& name);
	static bool check_class_end(InpArchive& inp);


	static void control_start(OutArchive &out);
	static void check_control_start(InpArchive &inp);

	static void mem_start(OutArchive& out, MemoryAlignmentType t);
	static void check_mem_start(InpArchive& inp, MemoryAlignmentType& t);
};

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

	static uint32_t hash24(const std::string& s)  {
		uint32_t hash = hash32(s);
		return (hash >> 24) ^ (hash & 0xFFFFFFU);
	}
};


}//namespace
