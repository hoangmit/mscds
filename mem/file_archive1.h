#pragma once

/**  \file

Implement Archive that save/load data to files. This archive load everything to 
memory.

This is version 1 archive layout (data and meta-data is mixed in one segment.)

*/

#ifndef __FILE_ARCHIVE_H_
#define __FILE_ARCHIVE_H_

#include "framework/archive.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

namespace mscds {

/// old file format, data region and meta-data is mixed
class OFileArchive1: public OutArchive {
public:
	OutArchive& startclass(const std::string& name, unsigned char version=1);

	OutArchive& endclass();
	OutArchive& save_bin(const void* ptr, size_t size);
	size_t opos() const;

	OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4);
	OutArchive& add_mem_region(const void* ptr, size_t size);
	OutArchive& end_mem_region();

	/// opens afile for writing
	void open_write(const std::string& fname);
	/// uses existing stream
	void assign_write(std::ostream * o);
	OFileArchive1();
	~OFileArchive1() {close();}
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

class IFileArchive1: public InpArchive {
public:
	IFileArchive1();
	~IFileArchive1() { close(); }

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS);

	size_t ipos() const;
	/// opens a file to read
	void open_read(const std::string& fname);
	/// uses existing stream to read
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


}//namespace


#endif //__FILE_ARCHIVE_H_
