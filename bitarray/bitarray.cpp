#include "bitarray.h"
#include <algorithm>
#include <sstream>


namespace mscds {


void BitArray::fillzero() {
	std::fill(data, data + word_count(), 0ull);
}

void BitArray::fillone() {
	std::fill(data, data + word_count(), ~0ull);
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

IArchive& BitArray::load_nocls(IArchive& ar) {
	ar.var("bit_len").load(bitlen);
	ptr = ar.var("bits").load_mem(0, sizeof(uint64_t) * word_count());
	data = (uint64_t*) ptr.get();
	return ar;
}

IArchive& BitArray::load(IArchive& ar) {
	ar.loadclass("Bitvector");
	load_nocls(ar);
	ar.endclass();
	return ar;
}

OArchive& BitArray::save_nocls(OArchive& ar) const {
	ar.var("bit_len").save(bitlen);
	ar.var("bits").save_bin(data, sizeof(uint64_t) * word_count());
	return ar;
}

OArchive& BitArray::save(OArchive& ar) const {
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

uint8_t BitArray::byte(size_t pos) const {
	assert(pos*8 < bitlen);
	return ((const uint8_t*) data)[pos];
}

//------------------------------------------------------------------------------
IArchive& FixedWArray::load(IArchive& ar) {
	ar.loadclass("FixedWArray");
	ar.var("bitwidth").load(width);
	b.load_nocls(ar);
	ar.endclass();
	return ar;
}
OArchive& FixedWArray::save(OArchive& ar) const {
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

BitArraySeqBuilder::BitArraySeqBuilder(size_t wordlen, OArchive &_ar): ar(_ar), pos(0) {
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
}

FixedWArray FixedWArray::build(const std::vector<unsigned int> &values) {
	unsigned int max_val = *max_element(values.begin(), values.end());
	unsigned int width = ceillog2(max_val + 1);
	FixedWArray out = create(values.size(), width);
	for (unsigned int i = 0; i < values.size(); ++i)
		out.set(i, values[i]);
	return out;
}


}//namespace
