#pragma once

/**  \file

Common functions to make and check archive.

*/

#include "framework/archive.h"

#include <string>
#include <stdint.h>

namespace mscds {

/// common file markers
class FileMarker {
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



}//namespace
