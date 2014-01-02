#include "fmap_archive.h"

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>


#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>

#include "impl/file_marker.h"
#include "local_mem.h"

namespace mscds {

using namespace std;
using namespace boost::interprocess;

struct FileMapImpl {
	file_mapping m_file;
	//std::vector<mapped_region> regions;
	std::ifstream fi;
	size_t pos;
};

void IFileMapArchive::open_read(const std::string &fname) {
	close();
	FileMapImpl * fm = new FileMapImpl();
	impl = fm;
	fm->fi.open(fname.c_str(), ios::binary);
	if (!fm->fi) throw ioerror("open file: " + fname);
	fm->pos = 0;
	fm->m_file = file_mapping(fname.c_str(), read_only);
}

void IFileMapArchive::close() {
	if (impl != NULL) {
		FileMapImpl * fm = (FileMapImpl *) impl;
		delete fm;
		impl = NULL;
	}
}

unsigned char IFileMapArchive::loadclass(const std::string &name) {
	FileMapImpl * fm = (FileMapImpl *) impl;
	if (!fm->fi) throw ioerror("stream error");
	return FileMaker::check_class_start(*this, name);
}


InpArchive& IFileMapArchive::load_bin(void *ptr, size_t size) {
	FileMapImpl * fm = (FileMapImpl *) impl;
	fm->fi.read((char*)ptr, size);
	fm->pos += size;
	return * this;
}

InpArchive &IFileMapArchive::endclass(){
	FileMaker::check_class_end(*this);
	return * this;
}

struct FMDeleter {
	mapped_region * ptr;
	FMDeleter() {}
	FMDeleter(const FMDeleter& other): ptr(other.ptr) {}
	FMDeleter(mapped_region * p):ptr(p){}

	void operator()(void* p) {
		delete ptr;
	}
};

StaticMemRegionPtr IFileMapArchive::load_mem_region() {
	FileMapImpl * fm = (FileMapImpl *)impl;
	MemoryAlignmentType align;
	FileMaker::check_mem_start(*this, align);
	uint32_t nsz;
	load_bin(&nsz, sizeof(nsz));
	fm->fi.seekg(nsz, ios_base::cur);
	std::shared_ptr<void> s;
	if (nsz > 0) {
		mapped_region * rg;
		rg = new mapped_region(fm->m_file, read_only, fm->pos, nsz);
		fm->pos += nsz;
		s = std::shared_ptr<void>(rg->get_address(), FMDeleter(rg));
	}
	LocalMemModel alloc;
	return alloc.adoptMem(nsz, s);
}

size_t IFileMapArchive::ipos() const {
	return ((FileMapImpl *) impl)->pos;
}

bool IFileMapArchive::eof() const {
	FileMapImpl * fm = (FileMapImpl *) impl;
	return fm->fi.eof();
}

}//namespace mscds 
