
#pragma once
#ifndef __ARCHIVE_H_
#define __ARCHIVE_H_

/** \file This file defines interfaces for Archive classes */

#include "mem_models.h"
#include "utils/endian.h"
#include <stdint.h>
#include <string>
#include <memory>
#include <stdexcept>


namespace mscds {

// DEPRECIATED
struct ArchiveProperties {
	bool writeable : 1 ; // out, save
	bool readable : 1;  // in, load
	bool direct_mem_api : 1;
	bool window_mem_api : 1;
	bool name_preserve : 1;

	unsigned int ext1;
	void * ext2;
};

/// IO error
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

/// Output Archive interface (to save data to disk)
class OutArchive {
public:
	virtual ~OutArchive() {}
	/// adds a name for annotation
	virtual OutArchive& var(const std::string&) { return * this; }
	virtual OutArchive& var(const char*) { return *this; }
	virtual OutArchive& annotate(const std::string&) { return * this; }

	/// starts a class
	virtual OutArchive& startclass(const std::string&, unsigned char version = 1) { return *this;  };
	virtual OutArchive& endclass() { return *this; };
	
	/// saves a primitive variable
	virtual OutArchive& save(uint32_t v) { return save_num(v); }
	virtual OutArchive& save(uint64_t v) { return save_num(v); }
	
    virtual OutArchive& save_num(uint64_t v);
	virtual OutArchive& save_bin(const void* ptr, size_t size) = 0;

	//--------------------------------------------------------------------
	/// save a memory region
	virtual OutArchive& save_mem_region(const void* ptr, size_t size);
	virtual OutArchive& save_mem(const StaticMemRegionAbstract& mem);

	/// save a memory region incrementally
	virtual OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4) = 0;
	virtual OutArchive& add_mem_region(const void* ptr, size_t size) = 0;
	virtual OutArchive& end_mem_region() = 0;
	
	/// closes the archive
	virtual void close() {}

	/// returns the stream position
	virtual size_t opos() const = 0;
private:
	static bool _copy_in(void* ct, const void*p, size_t len) {
		OutArchive* self = (OutArchive*)ct;
		self->add_mem_region(p, len);
		return true;
	}
};

/// Input archive interface
class InpArchive {
public:
	virtual ~InpArchive() {}
	/// variable name annotation (to be match with OutArchive)
	virtual InpArchive& var(const std::string&) { return *this; }
	virtual InpArchive& var(const char*) { return *this; }

	/// starts a class scope
	virtual unsigned char loadclass(const std::string& name) { return 0; };
	virtual InpArchive& endclass() { return *this; };

	/// load primitive variable
	virtual InpArchive& load(uint32_t& v) { uint64_t x; load_num(x); v = x; return *this; }
	virtual InpArchive& load(uint64_t& v) { load_num(v); return *this; }
	//virtual InpArchive& load(size_t& v) { return load_bin(&v, sizeof(v)); }
    virtual InpArchive& load_num(uint64_t &v);
	
	/// load values
	virtual InpArchive& load_bin(void* ptr, size_t size) = 0;

	// StaticMemRegion is defined in mem_models.h
	/// load memory region
	virtual StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS) = 0;
	
	/// current stream position
	virtual size_t ipos() const = 0;
	/// is end of file
	virtual bool eof() const = 0;
	/// close stream
	virtual void close() {}

	/// debug information
	virtual void inspect(const std::string& param, std::ostream& out) const {}
};


/// Interface for data structure to save/load from archive
struct SaveLoadInt {
	virtual ~SaveLoadInt() {}
	virtual void save(OutArchive& ar) const = 0;
	virtual void load(InpArchive& ar) = 0;
};

/// Interface for auxiliary data structure
template<typename T>
struct SaveLoadAuxInt {
	virtual ~SaveLoadAuxInt() {}
	virtual void save_aux(OutArchive& ar) const = 0;
	virtual void load_aux(InpArchive& ar, T t) = 0;

};

/// save a number to archive (with one byte for byte length and check sum)
inline OutArchive &OutArchive::save_num(uint64_t v) {
    uint8_t tag = 0, len = 0;
    if (v < 16) {
        tag = v;
        save_bin(&tag, 1);
    } else {
        uint64_t w = v;
        if (w > 0xFFFFFFFFull) {
            len += 4; w >>= 32; }
        if (w > 0xFFFF) {
            len += 2; w >>= 16; }
        if (w > 0xFF) {
            len += 1; w >>= 8; }
		if (w > 0) len += 1;
        assert(len <= 8);
        tag = (len << 4);
        w = v;
        while (w>15) w = (w>>4)+(w&15); // 4bit checksum
        tag |= w & 15;
        v = to_le64(v);
        save_bin(&tag, 1);
        save_bin(&v, len);
    }
    return *this;
}

inline InpArchive &InpArchive::load_num(uint64_t &v) {
    uint8_t tag = 0, len = 0;
    load_bin(&tag, 1);
    if (tag < 16) v = tag;
    else {
        len = tag >> 4;
        uint8_t chksum = tag & 15;
        v = 0;
        load_bin(&v, len);
        v = read_le64(v);
        uint64_t w = v;
        while (w>15) w = (w>>4)+(w&15); // 4bit checksum
        if (chksum != w)
			throw ioerror("integer read: checksum");
    }
    return *this;
}

inline OutArchive &OutArchive::save_mem_region(const void *ptr, size_t size) {
    start_mem_region(size); add_mem_region(ptr, size); return end_mem_region(); }

inline OutArchive &OutArchive::save_mem(const StaticMemRegionAbstract &mem) {
    start_mem_region(mem.size());
    if (mem.size() > 0) {
        if (FULL_MAPPING == mem.memory_type()) {
            add_mem_region(mem.get_addr(), mem.size());
        } else {
            mem.scan(0, mem.size(), OutArchive::_copy_in, this);
        }
    }
    end_mem_region();
    return *this;
}

}//namespace
#endif //__ARCHIVE_H_
