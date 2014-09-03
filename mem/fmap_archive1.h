#pragma once

/** \file

Archive to read data from file. This archive uses memory mapping to avoid loading all
data to memory at once.

This is version 1 archive layout (data and meta-data is mixed in one segment.)
*/

#ifndef __FILE_MAP_ARCHIVE_H_
#define __FILE_MAP_ARCHIVE_H_

#include "framework/mem_models.h"
#include "file_archive1.h"

namespace mscds {
struct FileMapImpl;

/// old archive format for file mapping input
class IFileMapArchive1: public InpArchive {
public:
	IFileMapArchive1(): impl(NULL) {}
	~IFileMapArchive1() {close();}

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	
	StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS);

	size_t ipos() const;

	void open_read(const std::string& fname);
	void close();
	bool eof() const;
private:
	FileMapImpl * impl;
};
}//namespace mscds 

#endif //__FILE_MAP_ARCHIVE_H_