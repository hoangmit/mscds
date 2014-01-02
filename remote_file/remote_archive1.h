
#pragma once

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
	bool has_direct_access() const { return false; }
	bool has_window_access() const { return true;  }
	MemoryAlignmentType alignment() const;

	~RemoteMem() {}

	unsigned int model_id() const { return 0; }
	size_t size() const { return sz; }
	void close() {}
	//direct access 
	const void* get_addr() const { return nullptr; }

	WindowMem get_window(size_t start, uint32_t len) const;

	void release_window(WindowMem& w) const;

	uint32_t max_win_size() const { return (uint32_t) file->max_map_size(); }

	//small one time access
	uint64_t getword(size_t wp) const { uint64_t val; read(wp*sizeof(uint64_t), sizeof(uint64_t), &val); return val; }
	char getchar(size_t i) const { char val; read(i, 1, &val); return val; }

	void setword(size_t wp, uint64_t val) { write(wp*sizeof(uint64_t), sizeof(uint64_t), &val); }
	void setchar(size_t i, char c) { write(i, 1, &c); }

	void read(size_t i, size_t rlen, void* dst) const;
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
	StaticMemRegionPtr load_mem_region();

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
