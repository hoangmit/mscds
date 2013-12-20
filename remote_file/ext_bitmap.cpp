#include "ext_bitmap.h"
#include "memmapfile.h"
#include <cassert>

typedef mman::MemoryMappedFile MappedFile;

ExternalBitMap::ExternalBitMap() : impl(nullptr), ptr(nullptr), _len(0), onecnt(0) {}
ExternalBitMap::~ExternalBitMap() {
	MappedFile * mmf = reinterpret_cast<MappedFile*>(impl);
	if (mmf != nullptr) delete mmf;
}

void ExternalBitMap::create(const std::string &path, size_t len, unsigned int extlen, const char *extinfo)
{
	//header 
	// -- identifier -- 4 bytes
	// -- ext len  -- 4 bytes
	// -- len      -- 8 bytes
	// -- onecount -- 8 bytes
	// -- dataptr  -- 8 bytes

	MappedFile * mmf = reinterpret_cast<MappedFile*>(impl);
	if (mmf != nullptr) delete mmf;
	mmf = new MappedFile();
	const unsigned int headersize = 8 * 4;
	const size_t extinfosize = ((extlen + 3) / 4) * 4;
	const size_t bitsize = (len + 7) / 8;
	mmf->create_rw(path, headersize + extinfosize + bitsize);
	char* start = (char*) mmf->addr;
	memcpy(start, "EBM_", 4);
	*((uint32_t*) (start + 4)) = extlen;
	*((uint64_t*) (start + 8)) = len;
	*((uint64_t*) (start + 16)) = 0;
	*((uint64_t*) (start + 24)) = headersize + extinfosize;
	memcpy(start + 32, extinfo, extlen);

	this->_len = len;
	this->ptr = start + headersize + extinfosize;
	this->extlen = extlen;
	for (size_t i = 0; i < bitsize; ++i) *ptr = 0;
	this->impl = mmf;
}

void ExternalBitMap::load(const std::string &path) {
	MappedFile * mmf = reinterpret_cast<MappedFile*>(impl);
	if (mmf != nullptr) delete mmf;
	mmf = new MappedFile();
	mmf->load_rw(path);
	char* start = (char*)mmf->addr;
	if (mmf->len < 4 || strncmp(start, "EBM_", 4) != 0) throw std::runtime_error("wrong file identifier");
	this->extlen = *((uint32_t*)(start + 4));
	this->_len = *((uint64_t*)(start + 8));
	this->onecnt = *((uint64_t*)(start + 16));
	this->ptr = start + *((uint64_t*)(start + 24));
	this->impl = mmf;
}

void ExternalBitMap::close() {
	MappedFile * mmf = reinterpret_cast<MappedFile*>(impl);
	if (mmf != nullptr) delete mmf;
	mmf = nullptr;
	_len = 0;
	onecnt = 0;
}

void ExternalBitMap::setbit(size_t p) {
	assert(ptr != nullptr);
	assert(p < _len);
	char * px = (ptr + p / 8);
	uint8_t mask = (1u << (p % 8));
	if (!(*px & mask)) {
		onecnt++;
		*px |= mask;
	}
}

void ExternalBitMap::clearbit(size_t p) {
	assert(ptr != nullptr);
	assert(p < _len);
	char * px = (ptr + p / 8);
	uint8_t mask = (1u << (p % 8));
	if (*px & mask) {
		onecnt--;
		*px &= (~mask);
	}
}

bool ExternalBitMap::getbit(size_t p) const {
	assert(ptr != nullptr);
	assert(p < _len);
	return  ((*(ptr + p / 8)) & (1 << (p % 8))) != 0;
}

size_t ExternalBitMap::length() const { return _len; }

size_t ExternalBitMap::one_count() const { return onecnt; }

const char *ExternalBitMap::get_extinfo() {
	MappedFile * mmf = reinterpret_cast<MappedFile*>(impl);
	if (mmf == nullptr) throw std::runtime_error("not open");
	char* start = (char*)mmf->addr;
	return start + 32;
}

