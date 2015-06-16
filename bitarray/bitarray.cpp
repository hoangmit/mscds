#include "bitarray.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <cstring>


namespace mscds {


//------------------------------------------------------------------------------
InpArchive& FixedWArray::load(InpArchive& ar) {
	ar.loadclass("FixedWArray");
	ar.var("bitwidth").load(width);
	b.load_nocls(ar);
	ar.endclass();
	return ar;
}
OutArchive& FixedWArray::save(OutArchive& ar) const {
	ar.startclass("FixedWArray", 1);
	ar.var("bitwidth").save(width);
	b.save_nocls(ar);
	ar.endclass();
	return ar;
}

std::string FixedWArray::to_str() const {
	std::ostringstream ss;
	ss << '{';
	if (length() > 0) ss << (*this)[0];
	for (unsigned int i = 1; i < length(); ++i)
		ss << ',' << (*this)[i];
	ss << '}';
	return ss.str();
}

/*
inline BitArray BitArray::clone_mem() const {
BitArray v(this->bitlen);
std::copy(data, data + word_count(), v.data);
return v;
}*/


template<typename T>
struct CppArrDeleter {
    void operator()(void* p) const { delete[]((T*)p); }
};

/*
BitArraySeqBuilder::BitArraySeqBuilder(size_t wordlen, OutArchive &_ar): ar(_ar), pos(0) {
	ar.startclass("Bitvector", 1);
	ar.var("bit_len").save(wordlen * 64);
	wl = wordlen;
}

void BitArraySeqBuilder::addword(uint64_t v) {
	ar.save_bin(&v, sizeof(v));
	pos++;
}

void BitArraySeqBuilder::done() {
	ar.endclass();
	assert(wl == pos);
}*/

void FixedWArrayBuilder::build_s(const std::vector<unsigned int> &values, FixedWArray* out) {
	out->clear();
	unsigned int max_val = *max_element(values.begin(), values.end());
	unsigned int width = ceillog2(max_val + 1);
	*out = create(values.size(), width);
	for (unsigned int i = 0; i < values.size(); ++i)
		out->set(i, values[i]);
}

mscds::BitArray::BitArray(size_t bitlen) {
    this->_bitlen = bitlen;
    size_t arrlen = (size_t)BitArray::ceildiv(bitlen, BitArray::WORDLEN);
    LocalMemAllocator alloc;
    _data = alloc.allocStaticMem(arrlen * sizeof(uint64_t));
    if (arrlen > 0) _data.setword(arrlen - 1, 0);
}

BitArray mscds::BitArrayBuilder::create(size_t bitlen) {
    BitArray v;
    if (bitlen == 0) return v;
    assert(bitlen > 0);
    size_t arrlen = (size_t)BitArray::ceildiv(bitlen, BitArray::WORDLEN);
    LocalMemAllocator alloc;
    v._data = alloc.allocStaticMem(arrlen * sizeof(uint64_t));
    v._bitlen = bitlen;
    if (arrlen > 0) v._data.setword(arrlen - 1, 0);
    return v;
}

BitArray mscds::BitArrayBuilder::create(size_t bitlen, const char *ptr) {
    BitArray v = create(bitlen);
    size_t bytelen = (size_t)BitArray::ceildiv(bitlen, 8);
    v._data._data.write(0, bytelen, (const void*)ptr);
    return v;
}

BitArray mscds::BitArrayBuilder::adopt(size_t bitlen, StaticMemRegionPtr p) {
    BitArray v;
    v._data = p;
    v._bitlen = bitlen;
    return v;
}


}//namespace
