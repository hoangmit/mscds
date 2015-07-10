
#pragma once

#include "bitarray.h"
#include <memory>

namespace mscds {
	
/// array of bytes
class ByteMemRange {
	std::shared_ptr<char> _ptr;
	unsigned int len;
	static void do_nothing_deleter(char*) {}
	static void array_deleter(char* p) {
		delete[] p;
	}
public:
	ByteMemRange(): len(0) {}
	ByteMemRange(ByteMemRange& m): _ptr(m._ptr), len(m.len) {}
	ByteMemRange(ByteMemRange&& m): _ptr(std::move(m._ptr)), len(m.len) {}

	template<typename T>
	static ByteMemRange ref(T& t, size_t len) {
		assert(len <= sizeof(T));
		ByteMemRange ret;
		ret._ptr = std::shared_ptr<char>((char*)(&t), do_nothing_deleter);
		ret.len = len;
		return ret;
	}
	template<typename T>
	static ByteMemRange ref(T& t) {
		return ref<T>(t, sizeof(t));
	}

	template<typename T>
	static ByteMemRange val(T v, size_t len) {
		char * p = new char[len];
		*((T*)p) = v;
		ByteMemRange ret;
		ret._ptr = std::shared_ptr<char>(p, array_deleter);
		ret.len = len;
		return ret;
	}
	template<typename T>
	static ByteMemRange val(T v) {
		return val(v, sizeof(v));
	}

	static ByteMemRange val_c(char v) {
		return val<char>(v, 1);
	}

	size_t length() const {
		return len;
	}

	char* ptr() {
		return _ptr.get();
	}
	const char* ptr() const {
		return _ptr.get();
	}

};


/// slice of a BitArray
struct BitRange {
	BitRange() : ba(nullptr), start(0), len(0) {}
	BitRange(const BitRange& other) : ba(other.ba), start(other.start), len(other.len) {}
	BitRange(BitArray* ba_, size_t start_, size_t len_) : ba(ba_), start(start_), len(len_) {}
	BitRange(const BitArrayInterface* ba_, size_t start_, size_t len_): ba(ba_), start(start_), len(len_) {}

	uint64_t bits(size_t start_, size_t len_) const {
		assert(start + start_ + len_ <= start + len);
		return ba->bits(start + start_, len_);
	}

	uint8_t byte(unsigned int i = 0) const {
		return bits(8*i, 8);
	}

	uint64_t word(unsigned int i = 0) const {
		return bits(64*i, 64);
	}

	/*void setbits(size_t start_, uint64_t value, unsigned int len_) {
		assert(start_ <= start && start_ + len_ <= start + len);
		ba->setbits(start + start_, value, len_);
	}*/

	BitRange inc_front(unsigned int l) const {
		assert(l <= len);
		return BitRange(ba, start + l, len - l);
	}

	const BitArrayInterface* ba;
	size_t start, len;
};

}//namespace
