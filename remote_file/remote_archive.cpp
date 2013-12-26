
#include "remote_archive.h"
#include "mem/impl/file_marker.h"

namespace mscds {

RemoteArchive::RemoteArchive() {}

unsigned char RemoteArchive::loadclass(const std::string &name) {
	if (!file) throw ioerror("stream error");
	uint32_t hash = FNV_hash24(name);
	uint32_t v;
	file->read((char*)&v, sizeof(v));
	if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
	pos += sizeof(v);
	return v >> 24;
}

InpArchive &RemoteArchive::load_bin(void *ptr, size_t size) {
	file->read((char*)ptr, size);
	pos += size;
	return *this;
}

InpArchive &RemoteArchive::endclass() {
	return * this;
}

StaticMemRegionPtr RemoteArchive::load_mem_region() {
	uint32_t header = 0;// = 0x92492400u | align;
	file->read((char*)(&header), sizeof(header));
	if ((header >> 8) != 0x924924)
		throw ioerror("wrong mem_region start or corrupted data");
	MemoryAlignmentType align = (MemoryAlignmentType)(header & 0xFF);
	uint32_t nsz = 0;
	file->read((char*)(&nsz), sizeof(nsz));
	auto ret = std::make_shared<RemoteMem>();
	ret->file = this->file;
	ret->fstart = file->tellg();
	ret->sz = nsz;
	ret->ptr = nullptr;
	pos += nsz;
	file->seekg(ret->fstart + ret->sz);
	return StaticMemRegionPtr(ret);
}

size_t RemoteArchive::ipos() const {
	return pos;
}

void RemoteArchive::open_url(const std::string& url, const std::string& cache_dir, bool refresh) {
	if (file) {
		throw ioerror("close before open");
	}
	rep.change_cache_dir(cache_dir);
	file = rep.open(url, refresh);
	pos = 0;
}

void RemoteArchive::close() {
	file->close();
}

bool RemoteArchive::eof() const {
	return file->eof();
}

MemoryAlignmentType RemoteMem::alignment() const {
	return DEFAULT;
}

RemoteMem::WindowMem RemoteMem::get_window(size_t start, uint32_t len) const {
	WindowMem ret;
	if (file->has_mapping()) {
		ret.wid = 1;
		ret.ptr = file->create_map(start, len);
	} else {
		ret.wid = -1;
		char * px = new char[len];
		read(start, len, px);
		ret.ptr = px;
	}
	ret.wsize = len;
	return ret;
}

void RemoteMem::release_window(WindowMem &w) const {
	if (w.wid < 0) delete[](w.ptr);
}

void RemoteMem::read(size_t i, size_t rlen, void *dst) const {
	assert(i + rlen <= this->sz);
	file->seekg(i + fstart);
	file->read((char*)dst, rlen);
}

void RemoteMem::write(size_t i, size_t wlen, const void *dst) {
	throw ioerror("cannot write");
}


}
