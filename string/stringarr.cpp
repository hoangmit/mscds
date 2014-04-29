#include "stringarr.h"
#include "bitarray/bitstream.h"

#include <algorithm>

namespace mscds {


void StringArrBuilder::add(const std::string &s) {
	store.push_back(s);
}

void StringArrBuilder::build(StringArr *out) {
	out->clear();
	mscds::OBitStream os;
	mscds::SDArraySmlBuilder bd;
	out->tlen = 1;
	os.puts('\0', 8);
	size_t cp = 0;
	for (size_t i = 0; i < store.size(); i++) {
		const std::string& s = store[i];
		if (s.length() == 0) bd.add(0);
		else {
			bd.add(s.length() + 1);
			for (size_t j = 0; j < s.length(); ++j) 
				os.puts(s[j], sizeof(char)*8);
			os.puts('\0', 8);
			out->tlen += s.length() + 1;
		}
	}
	out->tlen += 1;
	os.puts('\0', 8);
	os.close();
	out->cnt = store.size();
	assert((os.length() + 63) / 64 == (out->tlen + 7) / 8);
	size_t arrlen = (out->tlen + 7)/ 8;
	out->ba = os.build();
	bd.build(&(out->start));

	if (out->ba.memory_type() != mscds::MAP_ON_REQUEST && out->ba.memory_type() != mscds::FULL_MAPPING) {
		out->mapping = false;
		out->ptrs = nullptr;
	}
	else {
		out->mapping = true;
		out->ptrs = (const char*)out->ba.get_addr();
	}
	store.clear();
}

void StringArrBuilder::build(mscds::OutArchive &ar) {
	StringArr out;
	build(&out);
	out.save(ar);
}

void StringArr::save(mscds::OutArchive &ar) const {
	ar.startclass("string_array", 1);
	ar.var("count").save(cnt);
	if (cnt > 0) {
		ar.var("length").save(tlen);
		start.save(ar.var("start"));
		ar.save_mem(ba);
		//ar.save_bin(ba.get(), ((tlen + 7) / 8)*8);
	}
	ar.endclass();
}

void StringArr::load(mscds::InpArchive &ar) {
	ar.loadclass("string_array");
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

void StringArr::clear() {
	cnt = tlen = 0;
	ptrs = NULL;
	ba.close();
	start.clear();
	mapping = false;
}


StringArr::StringArr() { clear(); }

struct MappingString: public StringInt {
	MappingString(char* p, size_t l, bool nf): ptr(p), len(l), req_free(nf),
		zerochar(0) {}
	MappingString(): ptr(&zerochar), zerochar(0), len(0), req_free(false) {}

	~MappingString() {
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

StringPtr StringArr::get(unsigned int i) const {
	assert(i < cnt);
	uint64_t ps = 0;
	uint64_t v = start.lookup(i, ps);
	if (v == 0)
		return std::make_shared<MappingString>(); // std::make_shared<MappingString>(ptrs + ps, 0, false);
	if (mapping) {
		ba.request_map(ps, v);
		return std::make_shared<MappingString>((char*)(ptrs + ps + 1), v - 1, false);
	} else {
		char *d = (char*)malloc(v);
		ba.read(ps + 1, v  - 1, d);
		return std::make_shared<MappingString>(d, v - 1, true);
	}
}

std::string StringArr::get_str(unsigned int i) const {
	assert(i < cnt);
	uint64_t ps = 0;
	uint64_t v = start.lookup(i, ps);
	if (v == 0) return "";
	if (mapping) {
		ba.request_map(ps, v);
		return std::string((ptrs + ps + 1), (ptrs + ps + v));
	} else {
		std::string st;
		st.reserve(v+1);
		ba.scan(ps + 1, v, [&st](const void * p, size_t len){
			st.append((const char*)p, len);
			return true;
		});
		st.resize(v-1);
		return st;
	}
}

}//namespace


