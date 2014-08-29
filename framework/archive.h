
#pragma once
#ifndef __ARCHIVE_H_
#define __ARCHIVE_H_

#include "mem_models.h"

#include <stdint.h>
#include <string>
#include <memory>
#include <stdexcept>


namespace mscds {

struct ArchiveProperties {
	bool writeable : 1 ; // out, save
	bool readable : 1;  // in, load
	bool direct_mem_api : 1;
	bool window_mem_api : 1;
	bool name_preserve : 1;

	unsigned int ext1;
	void * ext2;
};

class ioerror: public ::std::exception {
public:
	ioerror() {}
	~ioerror() throw() {}
	ioerror(const ioerror& other): msg(other.msg) {}
	ioerror(const std::string& _msg): msg(_msg) {}
	const char* what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

class OutArchive {
public:
	virtual ~OutArchive() {}
	virtual OutArchive& var(const std::string&) { return * this; }
	virtual OutArchive& var(const char*) { return *this; }
	virtual OutArchive& annotate(const std::string&) { return * this; }

	virtual OutArchive& startclass(const std::string&, unsigned char version = 1) { return *this;  };
	virtual OutArchive& endclass() { return *this; };
	
	virtual OutArchive& save(uint32_t v) { return save_bin(&v, sizeof(v)); }
	virtual OutArchive& save(int32_t v)  { return save_bin(&v, sizeof(v)); }
	virtual OutArchive& save(uint64_t v) { return save_bin(&v, sizeof(v)); }
	virtual OutArchive& save(int64_t v)  { return save_bin(&v, sizeof(v)); }
	//virtual OutArchive& save(size_t v) { return save_bin(&v, sizeof(v)); }
	virtual OutArchive& save_bin(const void* ptr, size_t size) = 0;

	//--------------------------------------------------------------------
	virtual OutArchive& save_mem_region(const void* ptr, size_t size);

	virtual OutArchive& save_mem(const StaticMemRegionAbstract& mem);

	virtual OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4) = 0;
	virtual OutArchive& add_mem_region(const void* ptr, size_t size) = 0;
	virtual OutArchive& end_mem_region() = 0;
	
	virtual void close() {}

	virtual size_t opos() const = 0;
	//virtual ArchiveProp properties() = 0;
};

class InpArchive {
public:
	virtual ~InpArchive() {}
	virtual InpArchive& var(const std::string&) { return *this; }
	virtual InpArchive& var(const char*) { return *this; }
	virtual unsigned char loadclass(const std::string& name) { return 0; };
	virtual InpArchive& endclass() { return *this; };

	virtual InpArchive& load(uint32_t& v) { return load_bin(&v, sizeof(v)); }
	virtual InpArchive& load(int32_t& v) { return load_bin(&v, sizeof(v)); }
	virtual InpArchive& load(uint64_t& v) { return load_bin(&v, sizeof(v)); }
	virtual InpArchive& load(int64_t& v) { return load_bin(&v, sizeof(v)); }
	//virtual InpArchive& load(size_t& v) { return load_bin(&v, sizeof(v)); }
	
	virtual InpArchive& load_bin(void* ptr, size_t size) = 0;

	// StaticMemRegion is defined in mem_models.h
	virtual StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS) = 0;
	
	virtual size_t ipos() const = 0;
	virtual bool eof() const = 0;
	virtual void close() {}
	virtual void inspect(const std::string& param, std::ostream& out) const {}
	//virtual ArchiveProp properties() = 0;
};

class SaveLoadInt {
	virtual void save(OutArchive& ar) const = 0;
	virtual void load(InpArchive& ar) = 0;
};

inline OutArchive &OutArchive::save_mem_region(const void *ptr, size_t size) {
	start_mem_region(size); add_mem_region(ptr, size); return end_mem_region(); }

inline OutArchive &OutArchive::save_mem(const StaticMemRegionAbstract &mem) {
	start_mem_region(mem.size());
	if (mem.size() > 0) {
		if (FULL_MAPPING == mem.memory_type()) {
			add_mem_region(mem.get_addr(), mem.size());
		}
		else {
			mem.scan(0, mem.size(), [this](const void*p, size_t len)->bool {
				this->add_mem_region(p, len);
				return true;
			});
		}
	}
	end_mem_region();
	return *this;
}

}//namespace
#endif //__ARCHIVE_H_
