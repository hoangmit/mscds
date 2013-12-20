#pragma once

#include <stdint.h>
#include <memory>
#include <cassert>
#include <string>

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

struct StaticMemRegionAbstract {
	virtual bool has_direct_access() const = 0;
	virtual bool has_window_access() const = 0;
	virtual MemoryAlignmentType alignment() const = 0;

	virtual ~StaticMemRegionAbstract() {}

	virtual unsigned int model_id() const { return 0; }
	virtual size_t size() const = 0;
	virtual void close() = 0;
	//direct access 
	virtual const void* get_addr() const = 0;
	
	//window access
	struct WindowMem {
		uint32_t wid;
		uint32_t wsize;
		char* ptr;
	};

	virtual WindowMem get_window(size_t start, uint32_t len) = 0;
	virtual void release_window(const WindowMem& w) = 0;
	virtual uint32_t max_win_size() = 0;

	//small one time access
	virtual uint64_t getword(size_t wp) const = 0;
	virtual char getchar(size_t i) const = 0;
	virtual void read(size_t i, size_t rlen, void* dst) const = 0;

	virtual void setword(size_t wp, uint64_t val) = 0;
	virtual void setchar(size_t i, char c) = 0;

	virtual void write(size_t i, size_t wlen, const void* dst) = 0;
};

struct DynamicMemRegionAbstract : public StaticMemRegionAbstract {
	virtual void resize(size_t size) = 0;
	virtual void append(char c) = 0;
	virtual void append(uint64_t word) = 0;
	virtual void append(const void * ptr, size_t len) = 0;
	//note that window may be invalidated after resize or append
};

//----------------------------------------------------------------------------


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

	bool has_direct_access() const { return _impl->has_direct_access(); }
	bool has_window_access() const { return _impl->has_window_access(); }
	MemoryAlignmentType alignment() const { return _impl->alignment(); }

	unsigned int model_id() const { return _impl->model_id(); }
	size_t size() const { return _impl->size(); }
	void close() { if (_impl != nullptr) { _impl->close(); _impl = nullptr; } }
	//direct access 
	const void* get_addr() const { return _impl->get_addr(); }

	//window access
	WindowMem get_window(size_t start, uint32_t len) { return _impl->get_window(start, len); }
	void release_window(const WindowMem& w) { _impl->release_window(w); }
	uint32_t max_win_size() { return _impl->max_win_size(); }


	//small one time access
	uint64_t getword(size_t wp) const { return _impl->getword(wp);}
	char getchar(size_t i) const { return _impl->getchar(i); }
	void read(size_t i, size_t rlen, void* dst) const { return _impl->read(i, rlen, dst); }

	void setword(size_t wp, uint64_t val) { _impl->setword(wp, val); }
	void setchar(size_t i, char c) { _impl->setchar(i, c); }

	void write(size_t i, size_t wlen, const void* dst) { _impl->write(i, wlen, dst); }
protected:
	StaticMemRegionAbstract * _impl;
	std::shared_ptr<StaticMemRegionAbstract> _ref;
};

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
	//--------------------------------------------------------------
	DynamicMemRegionPtr(const DynamicMemRegionPtr& mE) = default;
	DynamicMemRegionPtr& operator=(const DynamicMemRegionPtr& mE) = default;

	DynamicMemRegionPtr(DynamicMemRegionPtr&& mE) : _impl(mE._impl), _ref(std::move(mE._ref)) {}
	DynamicMemRegionPtr& operator=(DynamicMemRegionPtr&& mE) { _impl = mE._impl; _ref = std::move(_ref); }

	bool has_direct_access() const { return _impl->has_direct_access(); }
	bool has_window_access() const { return _impl->has_window_access(); }
	MemoryAlignmentType alignment() const { return _impl->alignment(); }

	unsigned int model_id() const { return _impl->model_id(); }
	size_t size() const { return _impl->size(); }
	void close() { _impl->close(); }
	//direct access 
	const void* get_addr() const { return _impl->get_addr(); }

	//page access
	WindowMem get_window(size_t start, uint32_t len) { return _impl->get_window(start, len); }
	void release_window(const WindowMem& w) { _impl->release_window(w); }
	uint32_t max_win_size() { return _impl->max_win_size(); }

	//small one time access
	uint64_t getword(size_t wp) const { return _impl->getword(wp); }
	char getchar(size_t i) const { return _impl->getchar(i); }
	void read(size_t i, size_t rlen, void* dst) const { return _impl->read(i, rlen, dst); }

	void setword(size_t wp, uint64_t val) { _impl->setword(wp, val); }
	void setchar(size_t i, char c) { _impl->setchar(i, c); }

	void write(size_t i, size_t wlen, const void* dst) { _impl->write(i, wlen, dst); }
protected:
	DynamicMemRegionAbstract * _impl;
	std::shared_ptr<DynamicMemRegionAbstract> _ref;
};

class MemoryModelAbstract {
public:
	virtual StaticMemRegionPtr allocStaticMem(size_t sz) = 0;
	virtual DynamicMemRegionPtr allocDynMem(size_t init_sz = 0) = 0;
	virtual StaticMemRegionPtr convert(const DynamicMemRegionAbstract& mem) = 0;
};

//--------------------------------------------------------------------------------


}//namespace
