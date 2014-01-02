#pragma once

#ifndef __FILE_MAP_ARCHIVE2_H_
#define __FILE_MAP_ARCHIVE2_H_

#include "framework/mem_models.h"
#include "file_archive.h"

namespace mscds {
struct FileMapImpl2;

class IFileMapArchive2: public InpArchive {
public:
	IFileMapArchive2(): impl(NULL) {}
	~IFileMapArchive2() {close();}

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	
	StaticMemRegionPtr load_mem_region();

	size_t ipos() const;

	void open_read(const std::string& fname);
	void close();
	bool eof() const;
private:
	FileMapImpl2 * impl;
};
}//namespace mscds 

#endif //__FILE_MAP_ARCHIVE2_H_