#pragma once

#include <cstdlib>
#include <string>
#include <stdint.h>

class ExternalBitMap {
public:
	ExternalBitMap();
	~ExternalBitMap();
	void create(const std::string& path, size_t len, unsigned int extlen = 0, const char* extinfo = nullptr);
	void load(const std::string& path);

	void close();
	
	bool getbit(size_t p) const;
	void setbit(size_t p);
	void clearbit(size_t p);
	void assignbit(size_t p, bool val) { if (val) setbit(p); else clearbit(p); }

	size_t length() const;
	size_t one_count() const;

	const char * get_extinfo();
	size_t get_ext_size() { return extlen; }
private:
	void* impl;

	size_t _len, onecnt;
	char* ptr;
	uint32_t extlen;
};

