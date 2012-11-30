#pragma once

#ifndef __FILE_ARCHIVE_H_
#define __FILE_ARCHIVE_H_

#include "archive.h"

#include <iostream>
#include <fstream>
#include <memory>

namespace mscds {

uint32_t FNV_hash32(const std::string& s);
uint32_t FNV_hash24(const std::string& s);

class OFileArchive: public OArchive {
public:
	OArchive& startclass(const std::string& name, unsigned char version);

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
private:
	bool needclose;
	size_t pos;
	std::istream * in;
};

}//namespace


#endif //__FILE_ARCHIVE_H_
