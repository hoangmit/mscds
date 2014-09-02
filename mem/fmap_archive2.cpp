#include "fmap_archive2.h"

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

struct FileMapImpl2 {
	file_mapping m_file;
	std::ifstream fi;
	size_t data_start, control_start;
};

void IFileMapArchive2::open_read(const std::string &fname) {
	close();
	FileMapImpl2 * fm = new FileMapImpl2();
	impl = fm;

	fm->fi.open(fname.c_str(), ios::binary);
	if (!fm->fi) throw ioerror("open file: " + fname);
	FileMarker::HeaderBlock hd;
	fm->fi.read((char*)&hd, sizeof(hd));
	size_t dp, cp;
	FileMarker::check_file_header(hd, dp, cp);
	fm->data_start = 0 + dp;
	auto control_pos = 0 + cp;
	fm->fi.seekg(control_pos);
	FileMarker::check_control_start(*this);
	fm->control_start = fm->fi.tellg();
	fm->m_file = file_mapping(fname.c_str(), read_only);
}

void IFileMapArchive2::close() {
	if (impl != NULL) {
		FileMapImpl2 * fm = (FileMapImpl2 *) impl;
		fm->fi.close();
		delete fm;
		impl = NULL;
	}
}

unsigned char IFileMapArchive2::loadclass(const std::string &name) {
	FileMapImpl2 * fm = (FileMapImpl2 *) impl;
	if (!fm->fi) throw ioerror("stream error");
	return FileMarker::check_class_start(*this, name);
}


InpArchive& IFileMapArchive2::load_bin(void *ptr, size_t size) {
	FileMapImpl2 * fm = (FileMapImpl2 *) impl;
	fm->fi.read((char*)ptr, size);
	return * this;
}

InpArchive &IFileMapArchive2::endclass(){
	FileMarker::check_class_end(*this);
	return * this;
}

struct FMDeleter2 {
	mapped_region * ptr;
	FMDeleter2() {}
	FMDeleter2(const FMDeleter2& other): ptr(other.ptr) {}
	FMDeleter2(mapped_region * p):ptr(p){}

	void operator()(void* p) {
		delete ptr;
	}
};

StaticMemRegionPtr IFileMapArchive2::load_mem_region(MemoryAccessType mtp) {
	FileMapImpl2 * fm = (FileMapImpl2 *)impl;
	MemoryAlignmentType align;
	FileMarker::check_mem_start(*this, align);
	uint32_t nsz;
	load_bin(&nsz, sizeof(nsz));
	uint64_t ptrx;
	load_bin(&ptrx, sizeof(ptrx));
	std::shared_ptr<void> s;
	if (nsz > 0) {
		mapped_region * rg;
		rg = new mapped_region(fm->m_file, read_only, fm->data_start + ptrx, nsz);
		s = std::shared_ptr<void>(rg->get_address(), FMDeleter2(rg));
	}
	LocalMemAllocator alloc;
	return alloc.adoptMem(nsz, s);
}

size_t IFileMapArchive2::ipos() const {
	return 0;
}

bool IFileMapArchive2::eof() const {
	FileMapImpl2 * fm = (FileMapImpl2 *) impl;
	return fm->fi.eof();
}

}//namespace mscds 
