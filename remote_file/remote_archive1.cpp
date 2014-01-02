
#include "remote_archive1.h"
#include "mem/impl/file_marker.h"

namespace mscds {

RemoteArchive1::RemoteArchive1() {}

unsigned char RemoteArchive1::loadclass(const std::string &name) {
	if (!file) throw ioerror("stream error");
	return FileMarker::check_class_start(*this, name);
}

InpArchive &RemoteArchive1::load_bin(void *ptr, size_t size) {
	file->read((char*)ptr, size);
	pos += size;
	return *this;
}

InpArchive &RemoteArchive1::endclass() {
	FileMarker::check_class_end(*this);
	return * this;
}

StaticMemRegionPtr RemoteArchive1::load_mem_region() {
	MemoryAlignmentType align;
	FileMarker::check_mem_start(*this, align);
	uint32_t nsz = 0;
	load_bin(&nsz, sizeof(nsz));
	auto ret = std::make_shared<RemoteMem>();
	ret->file = this->file;
	ret->fstart = file->tellg();
	ret->sz = nsz;
	ret->ptr = nullptr;
	pos += nsz;
	file->seekg(ret->fstart + ret->sz);
	return StaticMemRegionPtr(ret);
}

size_t RemoteArchive1::ipos() const {
	return pos;
}

void RemoteArchive1::open_url(const std::string& url, const std::string& cache_dir, bool refresh) {
	if (file) {
		throw ioerror("close before open");
	}
	rep.change_cache_dir(cache_dir);
	file = rep.open(url, refresh);
	pos = 0;
}

void RemoteArchive1::close() {
	file->close();
}

bool RemoteArchive1::eof() const {
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
