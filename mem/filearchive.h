#pragma once

#ifndef __FILE_ARCHIVE_H_
#define __FILE_ARCHIVE_H_

#include "archive.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

namespace mscds {

uint32_t FNV_hash32(const std::string& s);
uint32_t FNV_hash24(const std::string& s);

class OFileArchive: public OArchive {
public:
	OArchive& startclass(const std::string& name, unsigned char version=1);

	OArchive& endclass();
	OArchive& save_bin(const void* ptr, size_t size);
	size_t opos() const;

	void open_write(const std::string& fname);
	void assign_write(std::ostream * o);
	OFileArchive(): out(NULL), needclose(false),  openclass(0), closeclass(0) {}
	~OFileArchive() {close();}
	void close();
private:
	unsigned int openclass, closeclass;
	std::ostream * out;
	size_t pos;
	bool needclose;
};

class IFileArchive: public IArchive {
public:
	IFileArchive(): in(NULL), needclose(false), pos(0) {}
	~IFileArchive() {close();}

	unsigned char loadclass(const std::string& name);
	IArchive& load_bin(void *ptr, size_t size);
	IArchive& endclass();

	SharedPtr load_mem(int type, size_t size);
	size_t ipos() const;

	void open_read(const std::string& fname);
	void assign_read(std::istream * i);
	void close();
	bool eof() const;
private:
	bool needclose;
	size_t pos;
	std::istream * in;
};

class IMemArchive: public IArchive {
public:
private:
	std::stringstream ss;
};

class OMemArchive: public OArchive {
public:
private:
	std::stringstream ss;
};

class OSizeEstArchive: public OArchive {
public:
	OSizeEstArchive(): pos(0) {}
	OArchive& startclass(const std::string& name, unsigned char version=1) {return *this;}
	OArchive& endclass() {return *this;}
	OArchive& save_bin(const void* ptr, size_t size) { pos += size; return *this;}
	size_t opos() const {return pos;}
	void close() {}
private:
	uint64_t pos;
};

class OClassInfoArchive: public OArchive {
public:
	OClassInfoArchive();
	~OClassInfoArchive();
	OArchive& var(const std::string& name);
	OArchive& save_bin(const void* ptr, size_t size);
	OArchive& startclass(const std::string& name, unsigned char version=1);
	OArchive& endclass();
	void close();
	size_t opos() const {return pos;}
	std::string printxml();
private:
	uint64_t pos;
	bool finalized;
	void* impl;
};

void save_str(OArchive& ar, const std::string& st);
std::string load_str(IArchive& ar);

template<typename T>
inline void save_to_file(const T& a, const std::string& name) {
	OFileArchive ar;
	ar.open_write(name);
	a.save(ar);
	ar.close();
}

template<typename T>
inline size_t estimate_size(const T& a) {
	OSizeEstArchive ar;
	a.save(ar);
	ar.close();
	return ar.opos();
}

}//namespace


#endif //__FILE_ARCHIVE_H_
