#pragma once


#include "framework/archive.h"
#include "file_archive1.h"

#include <memory>
#include <sstream>

namespace mscds {

class IMemArchive;

class OMemArchive : public OFileArchive {
public:
	OMemArchive() : OFileArchive() {
		ss = std::make_shared<std::stringstream>(std::ios::binary | std::ios::in | std::ios::out);
		assign_write(ss.get());
	}
private:
	std::shared_ptr<std::stringstream> ss;
	OFileArchive ar;
	friend class IMemArchive;
};

class IMemArchive : public IFileArchive {
public:
	IMemArchive(OMemArchive& in) : IFileArchive() {
		this->ss = in.ss;
		assign_read(ss.get());
	}
private:
	std::shared_ptr<std::stringstream> ss;
};

class OSizeEstArchive: public OutArchive {
public:
	OSizeEstArchive(): pos(0) {}
	OutArchive& startclass(const std::string&, unsigned char version=1) {return *this;}
	OutArchive& endclass() {return *this;}
	OutArchive& save_bin(const void*, size_t size) { pos += size; return *this;}
	size_t opos() const {return pos;}

	OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4) { return *this; }
	OutArchive& add_mem_region(const void* ptr, size_t size)  { pos += size; return *this; }
	OutArchive& end_mem_region() { return *this; }
	void close() {}
private:
	uint64_t pos;
};

class OClassInfoArchive: public OutArchive {
public:
	OClassInfoArchive();
	~OClassInfoArchive();
	OutArchive& var(const std::string& name);
	OutArchive& save_bin(const void* ptr, size_t size);
	OutArchive& startclass(const std::string& name, unsigned char version=1);
	OutArchive& endclass();
	OutArchive& start_mem_region(size_t size, MemoryAlignmentType = A4);
	OutArchive& add_mem_region(const void* ptr, size_t size);
	OutArchive& end_mem_region();
	void close();
	size_t opos() const {return pos;}
	std::string printxml();
private:
	uint64_t pos;
	bool finalized;
	void* impl;
};

template<typename T>
inline size_t estimate_data_size(const T& a) {
	OSizeEstArchive ar;
	a.save(ar);
	ar.close();
	return ar.opos();
}

void save_str(OutArchive& ar, const std::string& st);
std::string load_str(InpArchive& ar);

}//namespace