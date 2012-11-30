#include "filearchive.h"
#include <iostream>
#include <cstring>

namespace mscds {

/* magic numbers from http://www.isthe.com/chongo/tech/comp/fnv/ */
static const uint32_t InitialFNV = 2166136261U;
static const uint32_t FNVMultiple = 16777619;

/* Fowler / Noll / Vo (FNV) Hash */
uint32_t FNV_hash32(const std::string& s) {
	uint32_t hash = InitialFNV;
	for(size_t i = 0; i < s.length(); i++) {
		hash = hash ^ (s[i]);       /* xor  the low 8 bits */
		hash = hash * FNVMultiple;  /* multiply by the magic number */
	}
	return hash;
}

uint32_t FNV_hash24(const std::string& s) {
	uint32_t hash = FNV_hash32(s);
	return (hash >> 24) ^ (hash & 0xFFFFFFU);
}

	OArchive& OFileArchive::startclass(const std::string& name, unsigned char version) {
		uint32_t v = FNV_hash24(name) | (((uint32_t)version) << 24);
		out->write((char*)&v, sizeof(v));
		pos += sizeof(v);
		openclass++;
		return * this;
	}

	OArchive& OFileArchive::endclass() { 
		out->write("cend", 4);
		closeclass++;
		pos+=4;
		return * this;
	}

	OArchive& OFileArchive::save_bin(const void* ptr, size_t size) {
		out->write((char*)ptr, size);
		pos += size;
		return *this;
	}

	size_t OFileArchive::opos() const {
		return pos;
	}

	void OFileArchive::open_write(const std::string& fname) {
		close();
		std::ofstream * fout = (new std::ofstream(fname.c_str(), std::ios::binary));
		if (!fout->is_open()) throw ioerror("cannot open file");
		out = fout;
		needclose = true;
		pos = 0;
		openclass = 0;
		closeclass = 0;
	}

	void OFileArchive::assign_write(std::ostream * o) {
		out = o;
		needclose = false;
		pos = 0;
		openclass = 0;
		closeclass = 0;
	}

	void OFileArchive::close() {
		if (openclass != closeclass) 
			std::cout << "Warning: startclass != endclass " << std::endl;

		if (out != NULL && needclose) {
			delete out;
			needclose = false;
		}
		out = NULL;
	}

//----------------------------------------------------------------------

	unsigned char IFileArchive::loadclass(const std::string& name) {
		uint32_t hash = FNV_hash24(name);
		//unsigned char version = 0;
		uint32_t v;
		in->read((char*)&v, sizeof(v));
		if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
		pos += sizeof(v);
		return v >> 24;
	}

	IArchive& IFileArchive::load_bin(void *ptr, size_t size) {
		in->read((char*)ptr, size);
		pos += size;
		return *this;
	}

	struct Deleter {
		void operator()(void* p) {
			operator delete (p);
		}
	};

	SharedPtr IFileArchive::load_mem(int type, size_t size) {
		void * p = operator new(size);
		in->read((char*)p, size);
		pos += size;
		return SharedPtr(p, Deleter());
	}

	size_t IFileArchive::ipos() const {
		return pos;
	}

	IArchive& IFileArchive::endclass() { 
		char buf[5];
		in->read(buf, 4);
		buf[4] = 0;
		if (strcmp(buf, "cend") != 0) throw ioerror("wrong endclass");
		pos += 4;
		return * this;
	}

	void IFileArchive::open_read(const std::string& fname) {
		close();
		std::ifstream * fin = new std::ifstream(fname.c_str(), std::ios::binary);
		if (!fin->is_open()) throw ioerror("cannot open file");
		in = fin;
		needclose = true;
		pos = 0;
	}

	void IFileArchive::assign_read(std::istream * i) {
		in = i;
		needclose = false;
		pos = 0;
	}

	void IFileArchive::close() {
		if (in != NULL && needclose) {
			delete in;
			needclose = false;
		}
		in = NULL;
	}
} //namespace mscds