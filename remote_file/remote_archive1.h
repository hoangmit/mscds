
#pragma once

/** 
Implement archive for remote file.

(Archive structure version 1.)

*/

#include "framework/mem_models.h"
#include "framework/archive.h"
#include "remote_file.h"

#include <string>

namespace mscds {
class RemoteArchive1;
class RemoteArchive2;

class RemoteMem : public StaticMemRegionAbstract {
public:
	RemoteMem() : ptr(nullptr), sz(0) {}
	
	MemoryAlignmentType alignment() const;

	~RemoteMem() {}

	unsigned int model_id() const { return 0; }
	size_t size() const { return sz; }
	void close() {}

	const void* get_addr() const { return nullptr; }
	MemoryAccessType memory_type() const { return API_ACCESS; }
	bool request_map(size_t start, size_t len) { return false; }

	//small one time access
	uint64_t getword(size_t wp) const { uint64_t val; read(wp*sizeof(uint64_t), sizeof(uint64_t), &val); return val; }
	char getchar(size_t i) const { char val; read(i, 1, &val); return val; }

	void setword(size_t wp, uint64_t val) { write(wp*sizeof(uint64_t), sizeof(uint64_t), &val); }
	void setchar(size_t i, char c) { write(i, 1, &c); }

	void read(size_t i, size_t rlen, void* dst) const;
	void scan(size_t i, size_t len, CallBack cb) const;
	void write(size_t i, size_t wlen, const void* dst);
private:
	RemoteFileHdl file;
	size_t fstart, sz;
	char* ptr;
	friend class RemoteArchive1;
	friend class RemoteArchive2;
};

class RemoteArchive1 : public InpArchive {
public:
	RemoteArchive1();
	~RemoteArchive1() { close(); }

	unsigned char loadclass(const std::string& name);
	InpArchive& load_bin(void *ptr, size_t size);
	InpArchive& endclass();
	StaticMemRegionPtr load_mem_region(MemoryAccessType mtp = API_ACCESS);

	size_t ipos() const;

	void open_url(const std::string& url, const std::string& cache_dir = "", bool refresh = false);
	void close();
	bool eof() const;
private:
	RemoteFileRepository rep;
	RemoteFileHdl file;
	size_t pos;
};

}//namespace
