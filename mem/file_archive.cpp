#include "file_archive.h"
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

OutArchive& OFileArchive::startclass(const std::string& name, unsigned char version) {
	uint32_t v = FNV_hash24(name) | (((uint32_t)version) << 24);
	control->write((char*)&v, sizeof(v));
	pos += sizeof(v);
	openclass++;
	return * this;
}

OutArchive& OFileArchive::endclass() { 
	closeclass++;
	//out->write("cend", 4);
	//pos+=4;
	return * this;
}

OutArchive& OFileArchive::save_bin(const void* ptr, size_t size) {
	control->write((char*)ptr, size);
	pos += size;
	return *this;
}

size_t OFileArchive::opos() const {
	return pos;
}

OutArchive &OFileArchive::start_mem_region(size_t size, MemoryAlignmentType align) {
	if ((size >> 32) != 0) throw memory_error("unsupported big size mem");
	cur_mem_region = size;
	uint32_t header = 0x92492400u | align;
	control->write((char*)(&header), sizeof(header));
	uint32_t nsz = (uint32_t)size;
	control->write((char*)(&nsz), sizeof(nsz));
	return *this;
}

OutArchive &OFileArchive::add_mem_region(const void *ptr, size_t size) {
	if (size > cur_mem_region)
		throw ioerror("memory region data segment overflow");
	cur_mem_region -= size;
	data->write((const char*)ptr, size);
	return *this;
}

OutArchive &OFileArchive::end_mem_region() {
	if (cur_mem_region > 0) {
		throw ioerror("left over data");
	}
	return *this;
}

void OFileArchive::open_write(const std::string& fname) {
	close();
	std::ofstream * fout = (new std::ofstream(fname.c_str(), std::ios::binary));
	if (!fout->is_open()) throw ioerror("cannot open file to write: " + fname);
	const unsigned int BUFSIZE = 512 * 1024;
	if (buffer == NULL)
		buffer = new char[BUFSIZE];
	fout->rdbuf()->pubsetbuf(buffer, BUFSIZE);
	control = fout;
	data = fout;
	needclose = true;
	pos = 0;
	openclass = 0;
	closeclass = 0;
}

void OFileArchive::assign_write(std::ostream * o) {
	clear();
	control = o;
	data = o;
}

OFileArchive::OFileArchive(): control(NULL), data(NULL), needclose(false),  openclass(0), closeclass(0), buffer(NULL), cur_mem_region(0) {}

void OFileArchive::close() {
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

void OFileArchive::clear() {
	needclose = false;
	pos = 0;
	openclass = 0;
	closeclass = 0;
	cur_mem_region = 0;
}

//---------------------------------------------------------------------------

IFileArchive::IFileArchive(): control(NULL), data(NULL), needclose(false), pos(0), buffer(NULL) {}

unsigned char IFileArchive::loadclass(const std::string& name) {
	if (!control || !(*control)) throw ioerror("stream error");
	uint32_t hash = FNV_hash24(name);
	uint32_t v;
	control->read((char*)&v, sizeof(v));
	if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
	pos += sizeof(v);
	return v >> 24;
}

InpArchive& IFileArchive::load_bin(void *ptr, size_t size) {
	control->read((char*)ptr, size);
	pos += size;
	return *this;
}

StaticMemRegionPtr IFileArchive::load_mem_region() {
	uint32_t header=0;// = 0x92492400u | align;
	control->read((char*)(&header), sizeof(header));
	if ((header >> 8) != 0x924924)
		throw ioerror("wrong mem_region start or corrupted data");
	MemoryAlignmentType align = (MemoryAlignmentType) (header & 0xFF);
	uint32_t nsz = 0;
	control->read((char*)(&nsz), sizeof(nsz));
	LocalMemModel alloc;
	auto ret = alloc.allocStaticMem2(nsz);
	data->read((char*)(ret->get_addr()), nsz);
	pos += nsz;
	return StaticMemRegionPtr(ret);
}

size_t IFileArchive::ipos() const {
	return pos;
}

InpArchive& IFileArchive::endclass() { 
	return * this;
}

void IFileArchive::open_read(const std::string& fname) {
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

void IFileArchive::assign_read(std::istream * i) {
	control = i;
	data = i;
	needclose = false;
	pos = 0;
}

void IFileArchive::close() {
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

bool IFileArchive::eof() const {
	return control->eof();
}

//----------------------------------------------------------------------------
void save_str(OutArchive& ar, const std::string& st) {
	if (st.length() > 0xFFFF) throw ioerror("string too long");
	uint32_t v = (0x7374u << 16) | (st.length() & 0xFFFF); //"st"
	ar.save_bin(&v, sizeof(v));
	if (st.length() > 0)
		ar.save_bin(st.c_str(), st.length());
	v = 0;
	ar.save_bin(&v, 4 - (st.length() % 4));
}

std::string load_str(InpArchive& ar) {
	uint32_t v = 0;
	ar.load_bin(&v, sizeof(v));
	if ((v >> 16) != 0x7374u) throw ioerror("wrong string id");
	size_t len = v & 0xFFFF;
	char * st;
	st = new char[len + 1];
	if (len > 0) {
		ar.load_bin(st, len);
		st[len] = 0;
	}
	v = 0;
	ar.load_bin(&v, 4 - (len % 4));
	if (v != 0) { delete[] st; throw ioerror("wrong ending");}
	std::string ret(st, st + len);
	delete[] st;
	return ret;
}

} //namespace mscds
