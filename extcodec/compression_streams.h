#pragma once

/** 
\file

Implement wrapper of various external compression libraries.

Written by Hoang
*/

#include <stdint.h>
#include <iostream>
#include <vector>


namespace mscds {

struct ImplCompress {
	virtual ~ImplCompress() {}
	virtual std::ostream& out() = 0;
	virtual size_t streamsize() = 0;
	virtual void close() = 0;
};

enum ExtCompMethod { ZLIB, GZIP, BZIP2 };// 0 - zlib , 1- gzip, 2 - bzip2

class size_estimate_stream {
public:
	size_estimate_stream();
	size_estimate_stream(ExtCompMethod type);
	size_estimate_stream(const size_estimate_stream& other) : impl(other.impl) {}
	~size_estimate_stream();

	void push_printf(unsigned int, char sep = ' ');

	void init_stream(ExtCompMethod type);
	void write(const char* str, size_t len);

	void save(char c);
	void save(uint16_t);
	void save(uint32_t);
	void save(uint64_t);
	void close(); // return the size of the compressed stream
	size_t size() const;
private:
	size_t streamsize;
	ImplCompress* impl;
};

class comp_stream {
public:
private:
	std::vector<char> buffer;
};

}//namespace