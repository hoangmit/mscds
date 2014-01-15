
#include "remote_archive2.h"
#include "mem/impl/file_marker.h"
#include <iostream>

namespace mscds {

RemoteArchive2::RemoteArchive2() {}

unsigned char RemoteArchive2::loadclass(const std::string &name) {
	if (!file) throw ioerror("stream error");
	return FileMarker::check_class_start(*this, name);
}

InpArchive &RemoteArchive2::load_bin(void *ptr, size_t size) {
	file->seekg(control_pos);
	file->read((char*)ptr, size);
	control_pos += size;
	return *this;
}

InpArchive &RemoteArchive2::endclass() {
	FileMarker::check_class_end(*this);
	return * this;
}

StaticMemRegionPtr RemoteArchive2::load_mem_region() {
	MemoryAlignmentType align;
	FileMarker::check_mem_start(*this, align);
	uint32_t nsz = 0;
	load_bin(&nsz, sizeof(nsz));
	uint64_t ptrx;
	load_bin(&ptrx, sizeof(ptrx));

	auto ret = std::make_shared<RemoteMem>();
	ret->file = this->file;
	ret->fstart = data_start + ptrx;
	ret->sz = nsz;
	ret->ptr = nullptr;
	file->seekg(ret->fstart + ret->sz);
	return StaticMemRegionPtr(ret);
}

size_t RemoteArchive2::ipos() const {
	return 0;
}

void RemoteArchive2::open_url(const std::string& url, const std::string& cache_dir, bool refresh) {
	if (file) {
		throw ioerror("close before open");
	}
	rep.change_cache_dir(cache_dir);
	file = rep.open(url, refresh);
	FileMarker::HeaderBlock hd;
	file->read((char*)&hd, sizeof(hd));
	size_t dpos, cpos;
	FileMarker::check_file_header(hd, dpos, cpos);
	size_t xpos = 0;
	data_start = xpos + dpos;
	control_pos = xpos + cpos;
	FileMarker::check_control_start(*this);
	control_start = control_pos;
}

void RemoteArchive2::close() {
	if (file) file->close();
}

bool RemoteArchive2::eof() const {
	file->seekg(control_pos);
	return file->eof();
}

void RemoteArchive2::inspect(const std::string &param, std::ostream &out) const {
	out << "control_segment_pos = " << control_pos - control_start << std::endl;
	file->inspect(param, out);
}


}
