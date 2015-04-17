#include "blob_array.h"

#include "bitarray/bitstream.h"

#include <algorithm>

namespace mscds {

void BlobArrBuilder::add(const std::string &s) {
	store.push_back(s);
}

void BlobArrBuilder::build(BlobArr *out) {
	out->clear();
	mscds::OBitStream os;
	mscds::SDArraySmlBuilder bd;
	out->tlen = 0;
	size_t cp = 0;
	for (size_t i = 0; i < store.size(); i++) {
		const std::string& s = store[i];
		bd.add(s.length());
		for (size_t j = 0; j < s.length(); ++j)
			os.puts(s[j], sizeof(char)*8);
		out->tlen += s.length();
	}
	os.close();
	out->cnt = store.size();
	assert((os.length() + 63) / 64 == (out->tlen + 7) / 8);
	out->ba = os.build();
	bd.build(&(out->start));

	if (out->ba.memory_type() != mscds::MAP_ON_REQUEST && out->ba.memory_type() != mscds::FULL_MAPPING) {
		out->mapping = false;
		out->ptrs = nullptr;
	} else {
		out->mapping = true;
		out->ptrs = (const char*)out->ba.get_addr();
	}
	store.clear();
}

void BlobArrBuilder::build(mscds::OutArchive &ar) {
	BlobArr out;
	build(&out);
	out.save(ar);
}

void BlobArr::save(mscds::OutArchive &ar) const {
	ar.startclass("blob_array");
	ar.var("count").save(cnt);
	if (cnt > 0) {
		ar.var("length").save(tlen);
		start.save(ar.var("start"));
		ar.save_mem(ba);
		//ar.save_bin(ba.get(), ((tlen + 7) / 8)*8);
	}
	ar.endclass();
}

void BlobArr::load(mscds::InpArchive &ar) {
	ar.loadclass("blob_array");
	ar.var("count").load(cnt);
	if (cnt > 0) {
		ar.var("length").load(tlen);
		start.load(ar.var("start"));
		ba = ar.load_mem_region(mscds::MAP_ON_REQUEST);
		if (ba.memory_type() != mscds::MAP_ON_REQUEST && ba.memory_type() != mscds::FULL_MAPPING) {
			mapping = false;
			ptrs = nullptr;
		} else {
			mapping = true;
			ptrs = (const char*)ba.get_addr();
			if (tlen > ba.size())
				throw mscds::memory_error("wrong size");
			//ba = ar.load_mem(0, ((tlen + 7) / 8)*8);
		}
	} else {
		ba.close();
		start.clear();
		tlen = 0;
		ptrs = nullptr;
	}
	ar.endclass();
}

void BlobArr::clear() {
	cnt = tlen = 0;
	ptrs = NULL;
	ba.close();
	start.clear();
	mapping = false;
}


BlobArr::BlobArr() { clear(); }

struct MappingBlob: public StringInt {
	MappingBlob(char* p, size_t l, bool nf): ptr(p), len(l), req_free(nf),
		zerochar(0) {}
	MappingBlob(): ptr(&zerochar), zerochar(0), len(0), req_free(false) {}

	~MappingBlob() {
		if (ptr != nullptr && req_free) {
			free(ptr);
			ptr = nullptr;
		}
	}

	const char* c_str() { return ptr; }
	size_t length() const { return len; }

	char* ptr;
	size_t len;
	bool req_free;
	char zerochar;
};

StringPtr BlobArr::get(unsigned int i) const {
	assert(i < cnt);
	uint64_t ps = 0;
	uint64_t v = start.lookup(i, ps);
	if (v == 0)
		return std::make_shared<MappingBlob>(); // std::make_shared<MappingString>(ptrs + ps, 0, false);
	if (mapping) {
		ba.request_map(ps, v);
		return std::make_shared<MappingBlob>((char*)(ptrs + ps), v, false);
	} else {
		char *d = (char*)malloc(v);
		ba.read(ps, v, d);
		return std::make_shared<MappingBlob>(d, v, true);
	}
}

std::string BlobArr::get_str(unsigned int i) const {
	assert(i < cnt);
	uint64_t ps = 0;
	uint64_t v = start.lookup(i, ps);
	if (v == 0) return "";
	if (mapping) {
		ba.request_map(ps, v);
		return std::string((ptrs + ps), (ptrs + ps + v));
	} else {
		std::string st;
		st.reserve(v);
		ba.scan(ps, v, [](void*ctx, const void * p, size_t len){
			std::string* st = (std::string*) ctx;
			st->append((const char*)p, len);
			return true;
		}, &st);
		st.resize(v);
		return st;
	}
}

}//namespace


