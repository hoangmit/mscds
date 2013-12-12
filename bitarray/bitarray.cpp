#include "bitarray.h"
#include <algorithm>
#include <sstream>


namespace mscds {


void BitArray::fillzero() {
	//std::fill(data, data + word_count(), 0ull);
	size_t wc = word_count();
	for (size_t i = 0; i < wc; ++i) data.setword(i, 0ull);
}

void BitArray::fillone() {
	//std::fill(data, data + word_count(), ~0ull);
	size_t wc = word_count();
	for (size_t i = 0; i < wc; ++i) data.setword(i, ~0ull);
}

uint64_t BitArray::count_one() const {
	if (bitlen == 0) return 0;
	uint64_t ret = 0;
	const uint64_t wc = bitlen / WORDLEN;
	for (size_t i = 0; i < wc; i++)
		ret += popcnt(word(i));
	for (size_t i = (bitlen / WORDLEN)*WORDLEN; i < bitlen; i++)
		if (bit(i)) ret++;
	return ret;
}

InpArchive& BitArray::load_nocls(InpArchive& ar) {
	ar.var("bit_len").load(bitlen);
	if (bitlen > 0) data = ar.load_mem_region();
	return ar;
}

InpArchive& BitArray::load(InpArchive& ar) {
	ar.loadclass("Bitvector");
	load_nocls(ar);
	ar.endclass();
	return ar;
}

OutArchive& BitArray::save_nocls(OutArchive& ar) const {
	ar.var("bit_len").save(bitlen);
	if (bitlen > 0) ar.var("bits").save_mem(data);
	return ar;
}

OutArchive& BitArray::save(OutArchive& ar) const {
	ar.startclass("Bitvector", 1);
	save_nocls(ar);
	ar.endclass();
	return ar;
}

std::string BitArray::to_str() const {
	assert(length() < (1UL << 16));
	std::string s;
	for (unsigned int i = 0; i < bitlen; ++i)
		if (bit(i)) s += '1';
		else s += '0';
	return s;
}

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

FixedWArray FixedWArray::build(const std::vector<unsigned int> &values) {
	unsigned int max_val = *max_element(values.begin(), values.end());
	unsigned int width = ceillog2(max_val + 1);
	FixedWArray out = create(values.size(), width);
	for (unsigned int i = 0; i < values.size(); ++i)
		out.set(i, values[i]);
	return out;
}


}//namespace
