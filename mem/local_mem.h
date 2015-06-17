#pragma once

/**  \file

Implement local memory model

*/

#include "framework/mem_models.h"
#include <cassert>
#include <vector>
#include <cstring>

namespace mscds {

class LocalMemModel;


/// local static RAM memory region
class LocalStaticMem : public StaticMemRegionAbstract {
public:
	LocalStaticMem(): len(0), ptr(nullptr) {}
	static const uint8_t WORDSZ = 8;
	bool has_direct_access() const { return true; }
	//MemoryAlignmentType alignment() const { return alignment_tp; }
	MemoryAlignmentType alignment() const { return A8; }
	bool is_writable() const { return true; }

	unsigned int model_id() const { return 1; }
	MemoryAccessType memory_type() const { return FULL_MAPPING; }
	const void* get_addr() const { return ptr; }
	bool request_map(size_t start, size_t len) { return true; }
	
	size_t size() const { return len; }

	uint64_t getword(size_t wp) const { assert(wp < (len + WORDSZ - 1) / WORDSZ); return *(((uint64_t*)ptr) + wp); }
	char getchar(size_t i) const { assert(i < len); return *(ptr + i); }
	void read(size_t i, size_t rlen, void* dest) const { assert(i + rlen <= len); memcpy(dest, ptr + i, rlen); }

	void setword(size_t wp, uint64_t val) { assert(wp < len / WORDSZ); *(((uint64_t*)ptr) + wp) = val; }
	void setchar(size_t i, char c) { assert(i < len); ptr[i] = c; }
	void write(size_t i, size_t wlen, const void* src) { assert(i + wlen <= len); memcpy(ptr + i, src, wlen); }
	bool is_unique_ref() const { return true; }
	void close() { clear(); }
	void clear() { len = 0; ptr = nullptr; _s.reset(); }
	void scan(size_t i, size_t rlen, CallBackPlain cb) const { assert(i + rlen <= len); cb(ptr + i, rlen); }
	void scan(size_t i, size_t rlen, CallBackContext cb, void* context) const { assert(i + rlen <= len); cb(context, ptr + i, rlen); }
	~LocalStaticMem() { close(); }
private:
	size_t len;
	char * ptr;
	std::shared_ptr<void> _s;
	//MemoryAlignmentType alignment_tp;
	friend class LocalMemAllocator;
	//friend class IFileArchive;
};

/// local dynamic RAM memory region
class LocalDynamicMem : public DynamicMemRegionAbstract {
public:
	static const uint8_t WORDSZ = 8;
	unsigned int model_id() const { return 2; }
	MemoryAccessType memory_type() const { return API_ACCESS; }
	MemoryAlignmentType alignment() const { return A8; }
	const void* get_addr() const { return data.data(); }
	bool request_map(size_t start, size_t len) { return true; }

	uint64_t getword(size_t wp) const { assert(wp < sz / WORDSZ); return data[wp]; }
	char getchar(size_t i) const { assert(i < sz); return *(((char*)data.data()) + i); }
	void read(size_t i, size_t rlen, void* dest) const { assert(i + rlen <= sz); char* ptr = ((char*)data.data()); memcpy(dest, ptr + i, rlen); }
	void scan(size_t i, size_t rlen, CallBackPlain cb) const { assert(i + rlen <= sz); char* ptr = ((char*)data.data()); cb(ptr + i, rlen); }
	void scan(size_t i, size_t rlen, CallBackContext cb, void* context) const { assert(i + rlen <= sz); char* ptr = ((char*)data.data()); cb(context, ptr + i, rlen); }

	void setword(size_t wp, uint64_t val) { assert(wp < sz / WORDSZ); data[wp] = val; }
	void setchar(size_t i, char c) { assert(i < sz);  char* ptr = ((char*)data.data()); *(ptr + i) = c; }
	void write(size_t i, size_t wlen, const void* src) { assert(i + wlen <= sz); char* ptr = ((char*)data.data());  memcpy(ptr + i, src, wlen); }

	void close() { sz = 0; data.clear(); }

	void clear() { sz = 0; data.clear(); }
	bool is_unique_ref() const { return true; }
	bool is_writable() const { return true; }

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

	void append(StaticMemRegionAbstract& other) {
		size_t oldsz = sz;
		size_t len = other.size();
		resize(len + sz);
		char* ptr = ((char*)data.data());
		other.read(0, len, ptr + oldsz);
	}

	size_t size() const { return sz; }

	void resize(size_t size) { if (size > 0) { sz = size; data.resize(1 + ((size - 1) / WORDSZ)); } else close(); }
	
	LocalDynamicMem() : sz(0) {}
	LocalDynamicMem(LocalDynamicMem&& other) : sz(other.sz), data(std::move(other.data)) {}
	LocalDynamicMem& operator=(LocalDynamicMem&& other) {
		sz = other.sz;
		data.clear();
		data.swap(other.data);
	}
private:
	size_t sz;
	std::vector<uint64_t> data;
	friend class LocalMemAllocator;
};

/// local RAM memory allocator
class LocalMemAllocator : public MemoryAllocatorAbstract {
public:
	struct Deleter {
		void operator()(void* p) {
			operator delete (p);
		}
	};

	StaticMemRegionPtr allocStaticMem(size_t size);
	std::shared_ptr<LocalStaticMem> allocStaticMem2(size_t size);
	StaticMemRegionPtr copy(const DynamicMemRegionAbstract& ptr);
	StaticMemRegionPtr move(DynamicMemRegionAbstract& ptr);
	DynamicMemRegionPtr allocDynMem(size_t init_sz = 0);
	std::shared_ptr<LocalDynamicMem> allocDynMem2(size_t init_sz = 0);
	StaticMemRegionPtr adoptMem(size_t size, std::shared_ptr<void> s);
};

inline StaticMemRegionPtr LocalMemAllocator::allocStaticMem(size_t size) {
	return StaticMemRegionPtr(allocStaticMem2(size));
}

inline std::shared_ptr<LocalStaticMem> LocalMemAllocator::allocStaticMem2(size_t size) {
	void * p = operator new(size);
	auto ret = std::make_shared<LocalStaticMem>();
	ret->ptr = (char*)p;
	ret->_s = std::shared_ptr<void>(p, Deleter());
	ret->len = size;
	return ret;
}

inline StaticMemRegionPtr LocalMemAllocator::copy(const DynamicMemRegionAbstract &ptr) {
	auto ret = allocStaticMem2(ptr.size());
	ptr.read(0, ptr.size(), ret->ptr);
	return StaticMemRegionPtr(ret);
}

inline StaticMemRegionPtr LocalMemAllocator::move(DynamicMemRegionAbstract& ptr) {
	LocalDynamicMem* dt = dynamic_cast<LocalDynamicMem*>(&ptr);
	if (dt != nullptr) {
		auto ret = std::make_shared<LocalStaticMem>();
		auto x = std::make_shared<std::vector<uint64_t>>();
		ret->len = ptr.size();
		x.get()->swap(dt->data);
		ret->ptr = (char*) x->data();
		ret->_s = x;
		dt->clear();
		return StaticMemRegionPtr(ret);
	} else {
		// cast failed
		auto ret = allocStaticMem2(ptr.size());
		ptr.read(0, ptr.size(), ret->ptr);
		ptr.clear();
		return StaticMemRegionPtr(ret);
	}
}

inline DynamicMemRegionPtr LocalMemAllocator::allocDynMem(size_t init_sz) {
	return DynamicMemRegionPtr(allocDynMem2(init_sz));
}

inline std::shared_ptr<LocalDynamicMem> LocalMemAllocator::allocDynMem2(size_t init_sz) {
	auto ret = std::make_shared<LocalDynamicMem>();
	ret->resize(init_sz);
	return ret;
}

inline StaticMemRegionPtr LocalMemAllocator::adoptMem(size_t size, std::shared_ptr<void> s) {
	auto ret = std::make_shared<LocalStaticMem>();
	ret->_s = s;
	ret->ptr = (char*)(s.get());
	ret->len = size;
	return StaticMemRegionPtr(ret);
}

}//namespace
