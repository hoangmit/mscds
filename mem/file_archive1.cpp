#include "file_archive1.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <tuple>
#include <stack>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "impl/file_marker.h"
#include "local_mem.h"

using namespace std;

namespace mscds {

OutArchive& OFileArchive1::startclass(const std::string& name, unsigned char version) {
	FileMarker::class_start(*this, name, version);
	openclass++;
	return * this;
}

OutArchive& OFileArchive1::endclass() { 
	FileMarker::class_end(*this);
	closeclass++;
	return * this;
}

OutArchive& OFileArchive1::save_bin(const void* ptr, size_t size) {
	control->write((char*)ptr, size);
	pos += size;
	return *this;
}

size_t OFileArchive1::opos() const {
	return pos;
}

OutArchive &OFileArchive1::start_mem_region(size_t size, MemoryAlignmentType align) {
	cur_mem_region = size;
	FileMarker::mem_start(*this, align);
	uint32_t sz = size;
	save_bin(&sz, sizeof(sz));
	return *this;
}

OutArchive &OFileArchive1::add_mem_region(const void *ptr, size_t size) {
	if (size > cur_mem_region)
		throw ioerror("memory region data segment overflow");
	cur_mem_region -= size;
	data->write((const char*)ptr, size);
	return *this;
}

OutArchive &OFileArchive1::end_mem_region() {
	if (cur_mem_region > 0) {
		throw ioerror("left over data");
	}
	return *this;
}

void OFileArchive1::open_write(const std::string& fname) {
	close();
	clear();
	std::ofstream * fout = (new std::ofstream(fname.c_str(), std::ios::binary));
	if (!fout->is_open()) throw ioerror("cannot open file to write: " + fname);
	const unsigned int BUFSIZE = 512 * 1024;
	if (buffer == NULL)
		buffer = new char[BUFSIZE];
	fout->rdbuf()->pubsetbuf(buffer, BUFSIZE);
	control = fout;
	data = fout;
	needclose = true;
}

void OFileArchive1::assign_write(std::ostream * o) {
	clear();
	control = o;
	data = o;
}

OFileArchive1::OFileArchive1(): control(NULL), data(NULL), needclose(false),  openclass(0), closeclass(0), buffer(NULL), cur_mem_region(0) {}

void OFileArchive1::close() {
	if (cur_mem_region > 0) throw ioerror("need close mem region");

	if (openclass != closeclass) 
		std::cout << "Warning: startclass != endclass " << std::endl;

	if (control != NULL && needclose) {
		delete control;
		needclose = false;
	}
	if (buffer != NULL) {
		delete[] buffer;
		buffer = NULL;
	}
	control = NULL;
	data = NULL;
}

void OFileArchive1::clear() {
	needclose = false;
	pos = 0;
	openclass = 0;
	closeclass = 0;
	cur_mem_region = 0;
}

//---------------------------------------------------------------------------

IFileArchive1::IFileArchive1(): control(NULL), data(NULL), needclose(false), pos(0), buffer(NULL) {}

unsigned char IFileArchive1::loadclass(const std::string& name) {
	if (!control || !(*control)) throw ioerror("stream error");
	return FileMarker::check_class_start(*this, name);
}

InpArchive& IFileArchive1::load_bin(void *ptr, size_t size) {
	control->read((char*)ptr, size);
	pos += size;
	return *this;
}

StaticMemRegionPtr IFileArchive1::load_mem_region() {
	MemoryAlignmentType align;
	FileMarker::check_mem_start(*this, align);
	uint32_t nsz;
	load_bin((char*)&nsz, sizeof(nsz));
	LocalMemModel alloc;
	auto ret = alloc.allocStaticMem2(nsz);
	data->read((char*)(ret->get_addr()), nsz);
	return StaticMemRegionPtr(ret);
}

size_t IFileArchive1::ipos() const {
	return pos;
}

InpArchive& IFileArchive1::endclass() {
	FileMarker::check_class_end(*this);
	return * this;
}

void IFileArchive1::open_read(const std::string& fname) {
	close();
	std::ifstream * fin = new std::ifstream(fname.c_str(), std::ios::binary);
	if (!fin->is_open()) throw ioerror("cannot open file to read: " + fname);
	const unsigned int BUFSIZE = 256 * 1024;
	if (buffer == NULL)
		buffer = new char[BUFSIZE];
	fin->rdbuf()->pubsetbuf(buffer, BUFSIZE);
	control = fin;
	data = fin;
	needclose = true;
	pos = 0;
}

void IFileArchive1::assign_read(std::istream * i) {
	control = i;
	data = i;
	needclose = false;
	pos = 0;
}

void IFileArchive1::close() {
	if (control != NULL && needclose) {
		delete control;
		needclose = false;
	}
	if (buffer != NULL) {
		delete[] buffer;
		buffer = NULL;
	}
	control = NULL;
}

bool IFileArchive1::eof() const {
	return control->eof();
}


} //namespace mscds
