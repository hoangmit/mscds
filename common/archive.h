
#pragma once
#ifndef __ARCHIVE_H_
#define __ARCHIVE_H_

#include <stdint.h>
#include <string>
#include <memory>

namespace mscds {

struct ArchiveProp {
	bool name_preserve;
	bool save_only;
	bool load_only;
	bool special_object_service;
	unsigned int ext1;
	void * ext2;
};

class ioerror: public std::exception {
public:
	ioerror() {}
	~ioerror() throw() {}
	ioerror(const ioerror& other): msg(other.msg) {}
	ioerror(const std::string& _msg): msg(_msg) {}
	const char* what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

class OArchive {
public:
	virtual ~OArchive() {}
	virtual OArchive& var(const std::string& name) { return * this; }
	virtual OArchive& annotate(const std::string& name) { return * this; }
	virtual OArchive& startclass(const std::string& name, unsigned char version) = 0;
	virtual OArchive& endclass() = 0;
	
	virtual OArchive& save(uint32_t v) { return save_bin(&v, sizeof(v)); }
	virtual OArchive& save(int32_t v) { return save_bin(&v, sizeof(v)); }

	virtual OArchive& save(uint64_t v) { return save_bin(&v, sizeof(v)); }
	virtual OArchive& save(int64_t v) { return save_bin(&v, sizeof(v)); }

	virtual OArchive& save_bin(const void* ptr, size_t size) = 0;
	virtual size_t opos() const = 0;
	//virtual ArchiveProp properties() = 0;
};

typedef std::shared_ptr<void> SharedPtr;

class IArchive {
public:
	virtual ~IArchive() {}
	virtual IArchive& var(const std::string& name) { return * this; }
	virtual unsigned char loadclass(const std::string& name) = 0;
	virtual IArchive& endclass() = 0;

	virtual IArchive& load(uint32_t& v) { return load_bin(&v, sizeof(v)); }
	virtual IArchive& load(int32_t& v) { return load_bin(&v, sizeof(v)); }

	virtual IArchive& load(uint64_t& v) { return load_bin(&v, sizeof(v)); }
	virtual IArchive& load(int64_t& v) { return load_bin(&v, sizeof(v)); }
	
	virtual IArchive& load_bin(void* ptr, size_t size) = 0;

	virtual SharedPtr load_mem(int type, size_t size) = 0;
	
	virtual size_t ipos() const = 0;
	//virtual ArchiveProp properties() = 0;
};


}//namespace
#endif //__ARCHIVE_H_
