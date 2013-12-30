#include "file_marker.h"

namespace mscds {


void FileMaker::class_start(OutArchive &out, const std::string &name, unsigned char version) {
	uint32_t v = FNV_hash::hash24(name) | (((uint32_t)version) << 24);
	out.save_bin((char*)&v, sizeof(v));
}

void FileMaker::class_end(OutArchive &out) {}

unsigned char FileMaker::check_class_start(InpArchive &inp, const std::string &name) {
	uint32_t hash = FNV_hash::hash24(name);
	uint32_t v;
	inp.load_bin(&v, sizeof(v));
	if ((v & 0xFFFFFF) != hash) throw ioerror("wrong hash");
	return v >> 24;
}

bool FileMaker::check_class_end(InpArchive &inp) {
	return true;
}

void FileMaker::mem_start(OutArchive &out, size_t size, MemoryAlignmentType align) {
	if ((size >> 32) != 0) throw memory_error("unsupported big size mem");
	uint32_t header = 0x92492400u | align;
	out.save_bin((char*)(&header), sizeof(header));
	uint32_t nsz = (uint32_t)size;
	out.save_bin((char*)(&nsz), sizeof(nsz));
}


size_t FileMaker::check_mem_start(InpArchive &inp, MemoryAlignmentType &t) {
	uint32_t header=0;// = 0x92492400u | align;
	inp.load_bin((char*)(&header), sizeof(header));

	if ((header >> 8) != 0x924924)
		throw ioerror("wrong mem_region start or corrupted data");
	MemoryAlignmentType align = (MemoryAlignmentType) (header & 0xFF);
	uint32_t nsz = 0;
	inp.load_bin((char*)(&nsz), sizeof(nsz));
	return nsz;
}



}
