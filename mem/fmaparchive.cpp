#include "fmaparchive.h"

#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

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
	uint32_t hash = FNV_hash24(name);
	//unsigned char version = 0;
	uint32_t v;
	fm->fi.read((char*)&v, sizeof(v));
	if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
	fm->pos += sizeof(v);
	return v >> 24;
}

IArchive &IFileMapArchive::load_bin(void *ptr, size_t size) {
	FileMapImpl * fm = (FileMapImpl *) impl;
	fm->fi.read((char*)ptr, size);
	fm->pos += size;
	return * this;
}

IArchive &IFileMapArchive::endclass(){
	FileMapImpl * fm = (FileMapImpl *) impl;
	char buf[5];
	fm->fi.read(buf, 4);
	buf[4] = 0;
	if (strcmp(buf, "cend") != 0) throw ioerror("wrong endclass");
	fm->pos += 4;
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

SharedPtr IFileMapArchive::load_mem(int type, size_t size) {
	FileMapImpl * fm = (FileMapImpl *) impl;
	fm->fi.seekg(size, ios_base::cur);
	if (size > 0) {
		mapped_region * rg;
		rg = new mapped_region(fm->m_file, read_only, fm->pos, size);
		fm->pos += size;
		return SharedPtr(rg->get_address(), FMDeleter(rg));
	} else {
		return SharedPtr();
	}
}

size_t IFileMapArchive::ipos() const {
	return ((FileMapImpl *) impl)->pos;
}

}//namespace mscds 