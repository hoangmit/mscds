#include "file_archive2.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <memory>
#include <tuple>
#include <stack>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstddef>

#include "impl/file_marker.h"
#include "local_mem.h"

using namespace std;

namespace mscds {

OutArchive& OFileArchive2::startclass(const std::string& name, unsigned char version) {
	FileMarker::class_start(*this, name, version);
	openclass++;
	return * this;
}

OutArchive& OFileArchive2::endclass() { 
	FileMarker::class_end(*this);
	closeclass++;
	return * this;
}

size_t OFileArchive2::opos() const {
	return sz_data + sz_control;
}

OutArchive& OFileArchive2::save_bin(const void* ptr, size_t size) {
	control.write((char*)ptr, size);
	sz_control += size;
	return *this;
}

OutArchive &OFileArchive2::start_mem_region(size_t size, MemoryAlignmentType align) {
	cur_mem_region = size;
	FileMarker::mem_start(*this, align);
	size_t s2 = size >> 16;
	s2 >>= 16;
	if (s2 > 0) throw ioerror("too big region");
	uint32_t sz = (uint32_t)size;
	save_bin(&sz, sizeof(sz));
	unsigned int al = memory_alignment_value(align);
	while (data.tellp() % al != 0) {
		data.put(0);
		sz_align_gap++;
	}
	uint64_t vx = sz_data + sz_align_gap;
	save_bin(&vx, sizeof(vx));
	return *this;
}

OutArchive &OFileArchive2::add_mem_region(const void *ptr, size_t size) {
	if (size > cur_mem_region)
		throw ioerror("memory region data segment overflow");
	cur_mem_region -= size;
	data.write((const char*)ptr, size);
	sz_data += size;
	return *this;
}

OutArchive &OFileArchive2::end_mem_region() {
	if (cur_mem_region > 0) {
		throw ioerror("left over data");
	}
	return *this;
}

void OFileArchive2::open_write(const std::string& fname) {
	close();
	clear();
	data.open(fname.c_str(), ios::binary);
	if (!data.is_open()) throw ioerror("cannot open file to write: " + fname);
	const unsigned int BUFSIZE = 512 * 1024;
	if (buffer == NULL)
		buffer = new char[BUFSIZE];
	data.rdbuf()->pubsetbuf(buffer, BUFSIZE);
	FileMarker::HeaderBlock hd;
	FileMarker::file_header(hd);
	data.write((char*)&hd, sizeof(hd));
	pointer_pos = 0 + offsetof(FileMarker::HeaderBlock, control_ptr);
	FileMarker::control_start(*this);
	sz_data = 0;
	sz_control = 0;
	sz_align_gap = 0;
}

OFileArchive2::OFileArchive2(): openclass(0), closeclass(0), buffer(NULL),
	cur_mem_region(0) {}

void OFileArchive2::post_process() {
	size_t cp = data.tellp();
	assert(cp == sz_data + sizeof(FileMarker::HeaderBlock) + sz_align_gap);
	// round to next multipel of 4
	size_t np = cp + 3 - (cp - 1) % 4;
	while (np > cp) {
		data.put(0);
		cp++;
	}
	data.seekp(pointer_pos);
	if (!data)
		throw std::runtime_error("not seekable");
	data.write((char*)&np, 8);
	data.seekp(np);
	data.write(control.str().data(), control.str().length());
}

void OFileArchive2::close() {
	if (cur_mem_region > 0) throw ioerror("need close mem region");

	if (openclass != closeclass) 
		std::cout << "Warning: startclass != endclass " << std::endl;

	if (buffer != NULL) {
		post_process();
		data.close();
		control.clear();
		control.str(std::string());
		delete[] buffer;
		buffer = NULL;
	}
}

void OFileArchive2::clear() {
	sz_data = 0;
	sz_control = 0;
	sz_align_gap = 0;
	openclass = 0;
	closeclass = 0;
	cur_mem_region = 0;
	control.str("");
	control.clear();
	data.close();
}

//---------------------------------------------------------------------------

IFileArchive2::IFileArchive2(): data(NULL), needclose(false), 
	buffer(NULL) {}

unsigned char IFileArchive2::loadclass(const std::string& name) {
	if (!data || !(*data)) throw ioerror("stream error");
	return FileMarker::check_class_start(*this, name);
}

InpArchive& IFileArchive2::load_bin(void *ptr, size_t size) {
	data->seekg(control_pos);
	data->read((char*)ptr, size);
	control_pos += size;
	return *this;
}

StaticMemRegionPtr IFileArchive2::load_mem_region(MemoryAccessType mtp) {
	MemoryAlignmentType align;
	FileMarker::check_mem_start(*this, align);
	uint32_t nsz;
	load_bin(&nsz, sizeof(nsz));
	uint64_t ptrx;
	load_bin(&ptrx, sizeof(ptrx));
	LocalMemModel alloc;
	auto ret = alloc.allocStaticMem2(nsz);
	data->seekg(data_start + ptrx);
	data->read((char*)(ret->get_addr()), nsz);
	return StaticMemRegionPtr(ret);
}

size_t IFileArchive2::ipos() const {
	return 0;
}

InpArchive& IFileArchive2::endclass() {
	FileMarker::check_class_end(*this);
	return * this;
}

void IFileArchive2::open_read(const std::string& fname) {
	close();
	std::ifstream * fin = new std::ifstream(fname.c_str(), std::ios::binary);
	if (!fin->is_open()) throw ioerror("cannot open file to read: " + fname);
	const unsigned int BUFSIZE = 256 * 1024;
	if (buffer == NULL)
		buffer = new char[BUFSIZE];
	fin->rdbuf()->pubsetbuf(buffer, BUFSIZE);
	data = fin;
	needclose = true;
	size_t dpos, cpos;
	size_t xpos = data->tellg();
	FileMarker::HeaderBlock hd;
	fin->read((char*)&hd, sizeof(hd));
	FileMarker::check_file_header(hd, dpos, cpos);
	data_start = xpos + dpos;
	control_pos = xpos + cpos;
	data->seekg(control_pos);
	FileMarker::check_control_start(*this);
	control_start = control_pos;
}

/*void IFileArchive2::assign_read(std::istream * i) {
	control = i;
	data = i;
	needclose = false;
	pos = 0;
}*/

void IFileArchive2::close() {
	if (data != NULL && needclose) {
		delete data;
		needclose = false;
		data = NULL;
	}
	if (buffer != NULL) {
		delete[] buffer;
		buffer = NULL;
	}
}

bool IFileArchive2::eof() const {
	data->seekg(control_pos);
	return data->eof();
}

void IFileArchive2::inspect(const std::string &param, std::ostream &out) const {
	out << "control_segment_pos = " << control_pos - control_start << std::endl;
}

} //namespace mscds
