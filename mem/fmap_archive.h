#pragma once

#ifndef __FILE_MAP_ARCHIVE_H_
#define __FILE_MAP_ARCHIVE_H_

#include "framework/mem_models.h"
#include "file_archive.h"

namespace mscds {
struct FileMapImpl;

class IFileMapArchive: public InpArchive {
public:
	IFileMapArchive(): impl(NULL) {}
	~IFileMapArchive() {close();}

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	
	StaticMemRegionPtr load_mem_region();

	size_t ipos() const;

	void open_read(const std::string& fname);
	void close();
	bool eof() const;
private:
	FileMapImpl * impl;
};
}//namespace mscds 

#endif //__FILE_MAP_ARCHIVE_H_