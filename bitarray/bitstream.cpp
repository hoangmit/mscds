#include "bitstream.h"

namespace mscds {

void mscds::OBitStream::append(const OBitStream &other) {
	if (!other.is_accessiable()) throw std::runtime_error("cannot extract");
	size_t px = 0, i = 0;
	while (px + WORDLEN < other.length()) {
		puts(other.os.getword(i));
		i++;
		px += WORDLEN;
	}
	if (px < other.length()) {
		uint64_t v = other.os.getword(i);
		puts(v, other.length() - px);
	}
}

void mscds::OBitStream::append(const BitArray &ba) {
	size_t n = ba.word_count();
	if (n == 0) return ;
	for (size_t i = 0; i < n - 1; ++i)
		puts(ba.word(i));
	auto m = ba.length() % 64;
	if (m == 0) m = 64;
	puts(ba.word(n - 1), m);
}

std::string mscds::OBitStream::to_str() const {
	std::ostringstream ss;
	for (int i = 0; i < bitlen; ++i)
		ss << ((getbit(i)) ? 1 : 0);
	return ss.str();
}

void OByteStream::puts(const std::string &str) {
	for (size_t i = 0; i < str.length(); ++i) os.append(str[i]);
}

void OByteStream::puts(const char *str, unsigned int len) {
	for (size_t i = 0; i < len; ++i) os.append(*(str + i));
}

void OByteStream::build(BitArray *out) {
	LocalMemAllocator alloc;
	*out = BitArrayBuilder::adopt(os.size() * 8, alloc.move(os));
}



}
