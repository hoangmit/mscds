#pragma once

#ifndef __FILE_ARCHIVE2_H_
#define __FILE_ARCHIVE2_H_

#include "framework/archive.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

namespace mscds {


class OFileArchive2: public OutArchive {
public:
	OutArchive& startclass(const std::string& name, unsigned char version=1);

	OutArchive& endclass();
	OutArchive& save_bin(const void* ptr, size_t size);
	size_t opos() const;

	OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4);
	OutArchive& add_mem_region(const void* ptr, size_t size);
	OutArchive& end_mem_region();

	void open_write(const std::string& fname);
	OFileArchive2();
	~OFileArchive2() {close();}
	void close();
private:
	void clear();
	size_t cur_mem_region;
	unsigned int openclass, closeclass;

	void post_process();

	std::ostringstream control;
	std::ofstream data;
	uint64_t sz_data, sz_control;
	uint32_t sz_align_gap;
	size_t pointer_pos;

	char * buffer;
};

class IFileArchive2: public InpArchive {
public:
	IFileArchive2();
	IFileArchive2(OFileArchive2&);
	~IFileArchive2() { close(); }

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS);

	size_t ipos() const;

	void open_read(const std::string& fname);
	//void assign_read(std::istream * i);
	void close();
	bool eof() const;
	void inspect(const std::string& param, std::ostream& out) const;
private:
	bool needclose;
	std::istream * data;
	size_t data_start, control_pos, control_start;
	char * buffer;
};


template<typename T>
inline void save_to_file(const T& a, const std::string& name) {
	OFileArchive2 ar;
	ar.open_write(name);
	a.save(ar);
	ar.close();
}


}//namespace


#endif //__FILE_ARCHIVE2_H_
