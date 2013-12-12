#include "stringarr.h"
#include "bitarray/bitstream.h"

#include <algorithm>

namespace app_ds {


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
	//out->ba = BitArray::create(os.data_ptr(), arrlen);
	//UNDONE
	//std::copy(, os.data_ptr() + arrlen, (uint64_t*)out->ba.get());
	bd.build(&(out->start));
	//out->ptrs = (const char*) out->ba.get();
	store.clear();
}

void StringArrBuilder::build(mscds::OArchive &ar) {
	StringArr out;
	build(&out);
	out.save(ar);
}

void StringArr::save(mscds::OArchive &ar) const {
	ar.startclass("string_array", 1);
	ar.var("count").save(cnt);
	if (cnt > 0) {
		ar.var("length").save(tlen);
		start.save(ar.var("start"));
		ar.save_bin(ba.get(), ((tlen + 7) / 8)*8);
	}
	ar.endclass();
}

void StringArr::load(mscds::IArchive &ar) {
	ar.loadclass("string_array");
	ar.var("count").load(cnt);
	if (cnt > 0) {
		ar.var("length").load(tlen);
		start.load(ar.var("start"));
		ba = ar.load_mem(0, ((tlen + 7) / 8)*8);
	} else {
		ba.reset();
		start.clear();
		tlen = 0;
	}
	ar.endclass();
	ptrs = (const char*) ba.get();
}

void StringArr::clear() {
	cnt = tlen = 0;
	ptrs = NULL;
	ba.reset();
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


