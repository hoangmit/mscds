#pragma once

#include <string>
#include <stdint.h>
namespace mscds {

class FileMaker {
	static unsigned char class_start_marker_width();
	static uint64_t create_marker(const std::string& name, unsigned char version);
	static unsigned char check_start_class_marker(const std::string& name, uint64_t word);

	static unsigned char mem_region_start_marker_width();
	static uint64_t create_mem_region_start_marker(const std::string& name, unsigned char version);
	static bool check_mem_region_start_marker(uint64_t);
};

uint32_t FNV_hash32(const std::string& s);
uint32_t FNV_hash24(const std::string& s);

}//namespace