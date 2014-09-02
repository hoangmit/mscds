#pragma once

/**
Defines abstracted memory interfaces

*/

#include <stdint.h>
#include <memory>
#include <cassert>
#include <string>
#include <stdexcept>
#include <functional>

namespace mscds {

class memory_error : public std::exception {
public:
	memory_error() {}
	~memory_error() throw() {}
	memory_error(const memory_error& other) : msg(other.msg) {}
	memory_error(const std::string& _msg) : msg(_msg) {}
	const char* what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

//--------------------------------------------------------------------------------

enum MemoryAlignmentType { DEFAULT, A1, A2, A4, A8 };

/// byte order in a word
enum EndiannessType {
	/// Intel or ARM CPU
	LITTLE_ENDIAN_ACCESS, 
	// PowerPC CPU
	BIG_ENDIAN_ACCESS
};

inline unsigned int memory_alignment_value(MemoryAlignmentType t) {
	switch (t) {
	case DEFAULT: return 1;
	case A1: return 1;
	case A4: return 4;
	case A8: return 8;
	default:
		throw std::runtime_error("unknown value");
	}
	return 0;
}

enum MemoryAccessType { API_ACCESS = 0, MAP_ON_REQUEST, FULL_MAPPING };

/// Static size Memory Region Interface
struct StaticMemRegionAbstract {
	/// returns the alignment type of the region
	virtual MemoryAlignmentType alignment() const = 0;
	/// returns the memory access type
	virtual MemoryAccessType memory_type() const = 0;
	/// returns the original endinanness 
	virtual EndiannessType endianness_type() const { return LITTLE_ENDIAN_ACCESS; } //only little-endian is supported at the moment
	
	/// gets the address the region in memory. 
	/// This functions is only available in FULL_MAPPING and MAP_ON_REQUEST modes
	virtual const void* get_addr() const = 0;
	
	/// fills the data. only available in MAP_ON_REQUEST mode
	virtual bool request_map(size_t start, size_t len) = 0;

	virtual ~StaticMemRegionAbstract() {}

	/// UNIMPLEMENTED: returns the unique id for the region
	virtual unsigned int model_id() const { return 0; }
	
	/// returns size of the region in bytes
	virtual size_t size() const = 0;

	/// stops using the region, and releases memory if necessary
	virtual void close() = 0;
	
	/// small one time access function
	virtual uint64_t getword(size_t wp) const = 0;
	virtual char getchar(size_t i) const = 0;

	virtual void setword(size_t wp, uint64_t val) = 0;
	virtual void setchar(size_t i, char c) = 0;

	virtual void read(size_t i, size_t rlen, void* dst) const = 0;
	virtual void write(size_t i, size_t wlen, const void* dst) = 0;

	// scans the memory region using some call-back function
	// The call-back function should return true to continue, and false to break
	typedef std::function<bool(const void* p, size_t len)> CallBack;
	virtual void scan(size_t i, size_t len, CallBack cb) const = 0;
};

/// Dynamic size memory region interface
/// the grow and sink of the dynamic memory region is like a stack. 
/// The additional or removed data is always at the end.
struct DynamicMemRegionAbstract : public StaticMemRegionAbstract {
	/// if the new size is smaller than the current size, the tail of the the region
	/// is truncated. If the new size is bigger, undefined data is appended to the
	//// end of the region
	virtual void resize(size_t size) = 0;

	/// appends a character at the end of the region, and increase the size by one
	virtual void append(char c) = 0;

	/// appends a word to the end of the region
	virtual void append(uint64_t word) = 0;

	/// appends a memory address
	virtual void append(const void * ptr, size_t len) = 0;

	/// appends another memory region
	virtual void append(StaticMemRegionAbstract& other) = 0;
};

//----------------------------------------------------------------------------

/// Pointer to static memory region
class StaticMemRegionPtr : public StaticMemRegionAbstract {
public:
	StaticMemRegionPtr() : _impl(nullptr) {}
	StaticMemRegionPtr(std::shared_ptr<StaticMemRegionAbstract> ref) : _ref(ref) { _impl = ref.get();}
	StaticMemRegionPtr(StaticMemRegionAbstract* ptr) : _impl(ptr) {}

	StaticMemRegionPtr(const StaticMemRegionPtr& mE) = default;
	StaticMemRegionPtr& operator=(const StaticMemRegionPtr& mE) = default;

	StaticMemRegionPtr(StaticMemRegionPtr&& mE) : _impl(mE._impl), _ref(std::move(mE._ref)) {}
	StaticMemRegionPtr& operator=(StaticMemRegionPtr&& mE) { _impl = mE._impl; _ref = std::move(mE._ref); return *this; }

	~StaticMemRegionPtr() {}

	MemoryAlignmentType alignment() const { return _impl->alignment(); }
	MemoryAccessType memory_type() const { return _impl->memory_type(); }
	//direct access 
	const void* get_addr() const { return _impl->get_addr(); }
	bool request_map(size_t start, size_t len) { return _impl->request_map(start, len); }


	unsigned int model_id() const { return _impl->model_id(); }
	size_t size() const { return _impl->size(); }
	void close() { if (_impl != nullptr) { _impl->close(); _impl = nullptr; } }


	//small one time access
	uint64_t getword(size_t wp) const { return _impl->getword(wp);}
	char getchar(size_t i) const { return _impl->getchar(i); }
	void read(size_t i, size_t rlen, void* dst) const { return _impl->read(i, rlen, dst); }

	void setword(size_t wp, uint64_t val) { _impl->setword(wp, val); }
	void setchar(size_t i, char c) { _impl->setchar(i, c); }

	void write(size_t i, size_t wlen, const void* dst) { _impl->write(i, wlen, dst); }
	void scan(size_t i, size_t len, CallBack cb) const { _impl->scan(i, len, cb); }
protected:
	StaticMemRegionAbstract * _impl;
	std::shared_ptr<StaticMemRegionAbstract> _ref;
};

/// Pointer to dynamic memory region
class DynamicMemRegionPtr : public DynamicMemRegionAbstract {
public:
	DynamicMemRegionPtr() : _impl(nullptr) {}
	DynamicMemRegionPtr(std::shared_ptr<DynamicMemRegionAbstract> ref) : _ref(ref) { _impl = ref.get(); }
	DynamicMemRegionPtr(DynamicMemRegionAbstract* ptr) : _impl(ptr) {}
	~DynamicMemRegionPtr() {}
	void resize(size_t size) { _impl->resize(size); }
	void append(char c) { _impl->append(c); }
	void append(uint64_t word) { _impl->append(word); }
	void append(const void * ptr, size_t len) { _impl->append(ptr, len); }
	void append(StaticMemRegionAbstract& other) { _impl->append(other); }
	//--------------------------------------------------------------
	DynamicMemRegionPtr(const DynamicMemRegionPtr& mE) = default;
	DynamicMemRegionPtr& operator=(const DynamicMemRegionPtr& mE) = default;

	DynamicMemRegionPtr(DynamicMemRegionPtr&& mE) : _impl(mE._impl), _ref(std::move(mE._ref)) {}
	DynamicMemRegionPtr& operator=(DynamicMemRegionPtr&& mE) { _impl = mE._impl; _ref = std::move(_ref); return * this; }

	MemoryAlignmentType alignment() const { return _impl->alignment(); }
	MemoryAccessType memory_type() const { return _impl->memory_type(); }
	//direct access 
	const void* get_addr() const { return _impl->get_addr(); }
	bool request_map(size_t start, size_t len) { return _impl->request_map(start, len); }

	unsigned int model_id() const { return _impl->model_id(); }
	size_t size() const { return _impl->size(); }
	void close() { _impl->close(); }

	//small one time access
	uint64_t getword(size_t wp) const { return _impl->getword(wp); }
	char getchar(size_t i) const { return _impl->getchar(i); }
	void read(size_t i, size_t rlen, void* dst) const { return _impl->read(i, rlen, dst); }

	void setword(size_t wp, uint64_t val) { _impl->setword(wp, val); }
	void setchar(size_t i, char c) { _impl->setchar(i, c); }

	void write(size_t i, size_t wlen, const void* dst) { _impl->write(i, wlen, dst); }
	void scan(size_t i, size_t len, CallBack cb) const { _impl->scan(i, len, cb); }
protected:
	DynamicMemRegionAbstract * _impl;
	std::shared_ptr<DynamicMemRegionAbstract> _ref;
};

/// Allocator
class MemoryAllocatorAbstract {
public:
	/// allocates a static memory region
	virtual StaticMemRegionPtr allocStaticMem(size_t sz) = 0;

	/// allocates a dynamic memory region
	virtual DynamicMemRegionPtr allocDynMem(size_t init_sz = 0) = 0;

	/// converts dynamic memory to static memory
	virtual StaticMemRegionPtr convert(const DynamicMemRegionAbstract& mem) = 0;
};

//--------------------------------------------------------------------------------


}//namespace
