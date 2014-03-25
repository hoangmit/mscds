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
	if (out->ba.memory_type() != mscds::MAP_ON_REQUEST && out->ba.memory_type() != mscds::FULL_MAPPING)
		throw mscds::memory_error("not supported memory type");
	out->ptrs = (const char*) out->ba.get_addr();
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
		if (ba.memory_type() != mscds::MAP_ON_REQUEST && ba.memory_type() != mscds::FULL_MAPPING)
			throw mscds::memory_error("not supported memory type");
		//ba = ar.load_mem(0, ((tlen + 7) / 8)*8);
		ptrs = (const char*)ba.get_addr();
		if (tlen > ba.size())
			throw mscds::memory_error("wrong size");
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
}

size_t StringArr::str_len(unsigned int i) const {
	return start.lookup(i);
}

StringArr::StringArr() { clear(); }

const char *StringArr::get(unsigned int i) const {
	assert(i < cnt);
	uint64_t ps = 0;
	uint64_t v = start.lookup(i, ps);
	if (v == 0) return ptrs + ps;
	else return ptrs + ps + 1;
}


}//namespace


