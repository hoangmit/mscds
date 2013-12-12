#pragma once

#ifndef __FILE_ARCHIVE_H_
#define __FILE_ARCHIVE_H_

#include "framework/archive.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

namespace mscds {


class OFileArchive: public OutArchive {
public:
	OutArchive& startclass(const std::string& name, unsigned char version=1);

	OutArchive& endclass();
	OutArchive& save_bin(const void* ptr, size_t size);
	size_t opos() const;

	OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4);
	OutArchive& add_mem_region(const void* ptr, size_t size);
	OutArchive& end_mem_region();

	void open_write(const std::string& fname);
	void assign_write(std::ostream * o);
	OFileArchive();
	~OFileArchive() {close();}
	void close();
private:
	void clear();
	size_t cur_mem_region;
	unsigned int openclass, closeclass;
	std::ostream * control;
	std::ostream * data;
	size_t pos;
	bool needclose;
	char * buffer;
};

class IFileArchive: public InpArchive {
public:
	IFileArchive();
	~IFileArchive() { close(); }

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	StaticMemRegionPtr load_mem_region();

	size_t ipos() const;

	void open_read(const std::string& fname);
	void assign_read(std::istream * i);
	void close();
	bool eof() const;
private:
	bool needclose;
	size_t pos;
	std::istream * control;
	std::istream * data;
	char * buffer;
};

void save_str(OutArchive& ar, const std::string& st);
std::string load_str(InpArchive& ar);

template<typename T>
inline void save_to_file(const T& a, const std::string& name) {
	OFileArchive ar;
	ar.open_write(name);
	a.save(ar);
	ar.close();
}

}//namespace


#endif //__FILE_ARCHIVE_H_
