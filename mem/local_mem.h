#pragma once

#include "framework/mem_models.h"
#include <cassert>
#include <vector>
#include <cstring>

namespace mscds {

class LocalMemModel;

class LocalStaticMem : public StaticMemRegionAbstract {
public:
	LocalStaticMem() : len(0), ptr(nullptr) {}
	static const uint8_t WORDSZ = 8;
	bool has_direct_access() const { return true; }
	bool has_window_access() const { return true; }
	//MemoryAlignmentType alignment() const { return alignment_tp; }
	MemoryAlignmentType alignment() const { return A8; }

	unsigned int model_id() const { return 1; }
	const void* get_addr() const { return ptr; }
	size_t size() const { return len; }

	WindowMem get_window(size_t start, uint32_t len) const { 
		WindowMem m;
		assert(start + len <= this->len);
		m.ptr = ptr + start;
		m.wid = 0;
		m.wsize = len;
		return m;
	}
	void release_window(WindowMem& w) const {}
	uint32_t max_win_size() const { return (uint32_t) this->len; }

	uint64_t getword(size_t wp) const { assert(wp < (len + WORDSZ - 1) / WORDSZ); return *(((uint64_t*)ptr) + wp); }
	char getchar(size_t i) const { assert(i < len); return *(ptr + i); }
	void read(size_t i, size_t rlen, void* dest) const { assert(i + rlen <= len); memcpy(dest, ptr + i, rlen); }

	void setword(size_t wp, uint64_t val) { assert(wp < len / WORDSZ); *(((uint64_t*)ptr) + wp) = val; }
	void setchar(size_t i, char c) { assert(i < len); ptr[i] = c; }
	void write(size_t i, size_t wlen, const void* src) { assert(i + wlen <= len); memcpy(ptr + i, src, wlen); }
	void close() {
		len = 0;
		ptr = nullptr;
	}
	~LocalStaticMem() { close(); }
private:
	size_t len;
	char * ptr;
	std::shared_ptr<void> _s;
	//MemoryAlignmentType alignment_tp;
	friend class LocalMemModel;
	//friend class IFileArchive;
};

class LocalDynamicMem : public DynamicMemRegionAbstract {
public:
	static const uint8_t WORDSZ = 8;
	unsigned int model_id() const { return 2; }
	bool has_direct_access() const { return true; }
	bool has_window_access() const { return true; }
	MemoryAlignmentType alignment() const { return A8; }
	const void* get_addr() const { return data.data(); }

	WindowMem get_window(size_t start, uint32_t len) const {
		WindowMem m;
		assert(start + len <= size());
		m.ptr = ((char*)data.data()) + start;
		m.wid = 0;
		m.wsize = len;
		return m;
	}
	void release_window(WindowMem& w) const {}
	uint32_t max_win_size() const { return (uint32_t) size(); }

	uint64_t getword(size_t wp) const { assert(wp < sz / WORDSZ); return data[wp]; }
	char getchar(size_t i) const { assert(i < sz); return *(((char*)data.data()) + i); }
	void read(size_t i, size_t rlen, void* dest) const { assert(i + rlen <= sz); memcpy(dest, data.data() + i, rlen); }

	void setword(size_t wp, uint64_t val) { assert(wp < sz / WORDSZ); data[wp] = val; }
	void setchar(size_t i, char c) { assert(i < sz);  char* ptr = ((char*)data.data()); *(ptr + i) = c; }
	void write(size_t i, size_t wlen, const void* src) { assert(i + wlen <= sz); memcpy(data.data() + i, src, wlen); }

	void close() { sz = 0; data.clear(); }

	void clear() { sz = 0; data.clear(); }

	void append(char c) { }
	void append(uint64_t word) {
		if (sz % WORDSZ == 0) { data.push_back(word); sz += WORDSZ; }
		else append((void*)(&word), WORDSZ);
	}

	void append(const void * ptr, size_t len) {
		size_t oldsz = sz;
		resize(len + sz);
		write(oldsz, len, ptr);
	}
	size_t size() const { return sz; }

	void resize(size_t size) { if (size > 0) { sz = size; data.resize(1 + ((size - 1) / WORDSZ)); } else close(); }
	
	LocalDynamicMem() : sz(0) {}
	LocalDynamicMem(LocalDynamicMem&& other) : sz(other.sz), data(std::move(other.data)) {}
private:
	size_t sz;
	std::vector<uint64_t> data;
	friend class LocalMemModel;
};


class LocalMemModel : public MemoryModelAbstract {
public:
	struct Deleter {
		void operator()(void* p) {
			operator delete (p);
		}
	};

	StaticMemRegionPtr allocStaticMem(size_t size) {
		return StaticMemRegionPtr(allocStaticMem2(size));
	}

	std::shared_ptr<LocalStaticMem> allocStaticMem2(size_t size) {
		void * p = operator new(size);
		auto ret = std::make_shared<LocalStaticMem>();
		ret->ptr = (char*)p;
		ret->_s = std::shared_ptr<void>(p, Deleter());
		ret->len = size;
		return ret;
	}

	StaticMemRegionPtr convert(const DynamicMemRegionAbstract& ptr) {
		auto ret = allocStaticMem2(ptr.size());
		ptr.read(0, ptr.size(), ret->ptr);
		return StaticMemRegionPtr(ret);
	}

	DynamicMemRegionPtr allocDynMem(size_t init_sz = 0) {
		return DynamicMemRegionPtr(allocDynMem2(init_sz));
	}
	std::shared_ptr<LocalDynamicMem> allocDynMem2(size_t init_sz = 0) {
		auto ret = std::make_shared<LocalDynamicMem>();
		ret->resize(init_sz);
		return ret;
	}

	StaticMemRegionPtr adoptMem(size_t size, std::shared_ptr<void> s) {
		auto ret = std::make_shared<LocalStaticMem>();
		ret->_s = s;
		ret->ptr = (char*)(s.get());
		ret->len = size;
		return StaticMemRegionPtr(ret);
	}
};

}//namespace
