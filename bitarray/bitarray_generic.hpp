#pragma once

#include <stdint.h>
#include <cassert>
#include <memory>

namespace mscds {

/// BitArray
template<typename WordAccess>
struct BitArrayGeneric {
	const static unsigned int WORDLEN = 64;

	/** sets one bit at "bitindex" with "value" */
	void setbit(size_t bitindex, bool value);
	/** sets a few bits start at "bitindex" with length "len", the values of those bits
	are given by the "value" input word */
	void setbits(size_t bitindex, uint64_t value, unsigned int len);
	/** sets 64 bits start at 64*"pos" with the input word "val" */
	void setword(size_t pos, uint64_t val);
	/** fills the array */
	void fillzero();
	void fillone();
	/** clear the bitarray and free memory */
	void clear();

	/** returns the length of the array */
    size_t length() const { return _bitlen; }
	/** returns the number of words */
	size_t word_count() const;
	/** counts how many one inside the array */
	uint64_t count_one() const;

	/** read one bit */
	bool bit(size_t bitindex) const;
	/** reads "len" bits from the array start at "bitindex" */
	/** reads one bit (operator version) */
	bool operator[](size_t i) const { return bit(i); }
	/** reads "len" bits from the array start at "bitindex" */
	uint64_t bits(size_t bitindex, unsigned int len) const;
	/** reads one byte (8 bits) at "pos"*8 */
	uint8_t byte(size_t pos) const;
	/** reads one word (64 bits) */
	uint64_t word(size_t pos) const;
	/** returns the number of 1-bit in the word at `p` */
	uint8_t popcntw(size_t pos) const;
	/** reads 32 bits block */
	uint32_t get_uint32(size_t pos) const;

	/** reads 64-bits at ''bitindex'' location (optimized version) */
	uint64_t bits64(size_t bitindex) const;

	//--------------------------------------------------
	/// scans the BitArray for the next 1-bit, returns -1 if cannot find
	int64_t scan_bits(uint64_t start, uint32_t res) const;
	///optimized version of scan_bits with p=0
	int64_t scan_next(uint64_t start) const;
	int64_t scan_bits_slow(uint64_t start, uint32_t res) const;

	/// scans for 0-bit
	int64_t scan_zeros(uint64_t start, uint32_t res) const;
	int64_t scan_zeros_slow(uint64_t start, uint32_t res) const;

	//--------------------------------------------------
    BitArrayGeneric();
    BitArrayGeneric(const BitArrayGeneric& other) = default;
    BitArrayGeneric& operator=(const BitArrayGeneric& other) = default;
    BitArrayGeneric(BitArrayGeneric&& mE): _bitlen(mE._bitlen), _data(std::move(mE._data)) {}
    BitArrayGeneric& operator=(BitArrayGeneric&& mE) { _bitlen = mE._bitlen; _data = std::move(mE._data); return *this; }

    //StaticMemRegionPtr data_ptr() const { return _data; }

	// freeze BitArray, not allow modifying
	void freeze() {}
	bool is_frozen() const { return false; }

    ~BitArrayGeneric();

	/** load the BitArray from InpArchive */
	InpArchive& load(InpArchive& ar);
	/** save the BitArray to OutArchive */
	OutArchive& save(OutArchive& ar) const;
	OutArchive& save_nocls(OutArchive& ar) const;
	InpArchive& load_nocls(InpArchive& ar);
	/** convert to string for debug or display */
	std::string to_str() const;

	inline static uint64_t ceildiv(uint64_t a, uint64_t b) {
		/* return (a != 0 ? ((a - 1) / b) + 1 : 0); // overflow free version */
		return (a + b - 1) / b;
	}
//private:
	friend class BitArrayBuilder;
    size_t _bitlen;
    WordAccess _data;
};

//---------------------------------------------------------------------

template<typename WordAccess>
inline BitArrayGeneric<WordAccess>::BitArrayGeneric(): _bitlen(0) {}

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::clear() { _bitlen = 0; _data = StaticMemRegionPtr(); }

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::setword(size_t pos, uint64_t val) { assert(pos < word_count()); _data.setword(pos, val); }
template<typename WordAccess>
inline uint64_t BitArrayGeneric<WordAccess>::word(size_t pos) const { assert(pos < word_count()); return _data.getword(pos); }
template<typename WordAccess>
inline uint8_t BitArrayGeneric<WordAccess>::popcntw(size_t pos) const { return popcnt(_data.getword(pos)); }
template<typename WordAccess>
inline uint32_t BitArrayGeneric<WordAccess>::get_uint32(size_t pos) const {
	assert((pos+1)*32 <= length());
    if (pos % 2 == 0) return _data.getword(pos / 2) & 0xFFFFFFFFu;
    else return _data.getword(pos / 2) >> 32;
}
template<typename WordAccess>
inline size_t BitArrayGeneric<WordAccess>::word_count() const { return ceildiv(_bitlen, WORDLEN); }

//inline const uint64_t* BitArray::data_ptr() const { return data; }

template<typename WordAccess>
inline BitArrayGeneric<WordAccess>::~BitArrayGeneric() {}

template<typename WordAccess>
inline uint64_t BitArrayGeneric<WordAccess>::bits64(size_t bitindex) const {
    assert(bitindex + WORDLEN <= _bitlen);
	uint64_t i = bitindex / WORDLEN;
	unsigned int j = bitindex % WORDLEN;
	if (j == 0) return word(i);
	else return (word(i) >> j) | (word(i+1) << j);
}

template<typename WordAccess>
inline uint64_t BitArrayGeneric<WordAccess>::bits(size_t bitindex, unsigned int len) const {
	assert(len <= WORDLEN); // len > 0
    assert(bitindex + len <= _bitlen);
	if (len==0) return 0;
	uint64_t i = bitindex / WORDLEN;
	unsigned int j = bitindex % WORDLEN;
	//uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull;
	uint64_t mask = ((~0ull) >> (WORDLEN - len));
	if (j + len <= WORDLEN)
		return (word(i) >> j) & mask;
	else
		return (word(i) >> j) | ((word(i + 1) << (WORDLEN - j)) & mask);
}

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::setbits(size_t bitindex, uint64_t value, unsigned int len) {
	assert(len <= WORDLEN);
    assert(bitindex + len <= _bitlen);
	if (len == 0) return ;
	uint64_t i = bitindex / WORDLEN;
	unsigned int j = bitindex % WORDLEN;
	//uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull; // & (~0ull >> (WORDLEN - len))
	uint64_t mask = ((~0ull) >> (WORDLEN - len));
	value = value & mask;
	uint64_t v = (word(i) & ~(mask << j)) | (value << j);
    _data.setword(i, v);
	if (j + len > WORDLEN)
		setword(i+1, (word(i+1) & ~(mask >> (WORDLEN - j))) | (value >> (WORDLEN - j)));
}

template<typename WordAccess>
inline bool BitArrayGeneric<WordAccess>::bit(size_t bitindex) const {
    assert(bitindex < _bitlen);
	return (word(bitindex / WORDLEN) & (1ULL << (bitindex % WORDLEN))) != 0;
}

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::setbit(size_t bitindex, bool value) {
    assert(bitindex < _bitlen);
	uint64_t v = word(bitindex / WORDLEN);
	if (value) v |= (1ULL << (bitindex % WORDLEN));
	else v &= ~(1ULL << (bitindex % WORDLEN));
	setword(bitindex / WORDLEN, v);
}

template<typename WordAccess>
inline uint8_t BitArrayGeneric<WordAccess>::byte(size_t pos) const {
    assert(pos * 8 < length());
	/*uint64_t _word = this->word(pos / 8);
	return (uint8_t)((_word >> (8*(pos % 8))) & 0xFF);*/
	return _data.getchar(pos);
}

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::fillzero() {
	//std::fill(data, data + word_count(), 0ull);
	size_t wc = word_count();
    for (size_t i = 0; i < wc; ++i) _data.setword(i, 0ull);
}

template<typename WordAccess>
inline void BitArrayGeneric<WordAccess>::fillone() {
	//std::fill(data, data + word_count(), ~0ull);
	size_t wc = word_count();
    for (size_t i = 0; i < wc; ++i) _data.setword(i, ~0ull);
}

template<typename WordAccess>
inline uint64_t BitArrayGeneric<WordAccess>::count_one() const {
    if (_bitlen == 0) return 0;
	uint64_t ret = 0;
    const uint64_t wc = _bitlen / WORDLEN;
	for (size_t i = 0; i < wc; i++)
		ret += popcntw(i);
    for (size_t i = (_bitlen / WORDLEN)*WORDLEN; i < _bitlen; i++)
		if (bit(i)) ret++;
	return ret;
}

template<typename WordAccess>
inline int64_t BitArrayGeneric<WordAccess>::scan_bits(uint64_t start, uint32_t res) const {
	assert(start < length());
	uint64_t wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = this->word(wpos) >> (start & 63);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return selectword(word, res);
		res -= bitcnt;
		++wpos;
	}
	while (wpos < this->word_count()) {
		uint32_t bitcnt = popcntw(wpos);
		if (bitcnt > res) {
			uint64_t word = this->word(wpos);
			return (wpos << 6) - start + selectword(word, res);
		}
		res -= bitcnt;
		++wpos;
	}
	return -1;
}

template<typename WordAccess>
inline int64_t BitArrayGeneric<WordAccess>::scan_next(uint64_t start) const {
	assert(start < length());
	uint64_t wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = this->word(wpos) >> (start & 63);
		if (word != 0) return lsb_intr(word);
		++wpos;
	}
	while (wpos < this->word_count()) {
		uint64_t word = this->word(wpos);
		if (word != 0)
			return (wpos << 6) - start + lsb_intr(word);
		++wpos;
	}
	return -1;
}

template<typename WordAccess>
inline int64_t BitArrayGeneric<WordAccess>::scan_bits_slow(uint64_t start, uint32_t res) const {
	for (size_t i = start; i < length(); i++) {
		if (this->bit(i)) {
			if (res == 0) return i - start;
			else res--;
		}
	}
	return -1;
}

template<typename WordAccess>
inline int64_t BitArrayGeneric<WordAccess>::scan_zeros(uint64_t start, uint32_t res) const {
	uint64_t wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = ~(this->word(wpos));
		if (wpos + 1 == word_count() && length() % WORDLEN != 0)
			word &= (1ull << (length() % WORDLEN)) - 1;
		word >>= (start & 63);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return selectword(word, res);
		res -= bitcnt;
		++wpos;
	}
	while (wpos + 1 < this->word_count()) {
		uint32_t bitcnt = WORDLEN - popcntw(wpos);
		if (bitcnt > res) {
			uint64_t word = ~(this->word(wpos));
			return (wpos << 6) - start + selectword(word, res);
		}
		res -= bitcnt;
		++wpos;
	}
	if (wpos + 1 == this->word_count()) {
		uint64_t word = ~(this->word(wpos));
		if (length() % 64 != 0)
			word &= (1ull << (length() % 64)) - 1;
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return (wpos << 6) - start + selectword(word, res);
	}
	return -1;
}

template<typename WordAccess>
inline int64_t BitArrayGeneric<WordAccess>::scan_zeros_slow(uint64_t start, uint32_t res) const {
	for (size_t i = start; i < length(); i++) {
		if (!this->bit(i)) {
			if (res == 0) return i - start;
			else res--;
		}
	}
	return -1;
}

template<typename WordAccess>
inline InpArchive& BitArrayGeneric<WordAccess>::load_nocls(InpArchive& ar) {
    ar.var("bit_len").load(_bitlen);
    if (_bitlen > 0) 
		_data.load(ar.var("bits"));
	return ar;
}

template<typename WordAccess>
inline InpArchive& BitArrayGeneric<WordAccess>::load(InpArchive& ar) {
	ar.loadclass("Bitvector");
	load_nocls(ar);
	ar.endclass();
	return ar;
}

template<typename WordAccess>
inline OutArchive& BitArrayGeneric<WordAccess>::save_nocls(OutArchive& ar) const {
    ar.var("bit_len").save(_bitlen);
    if (_bitlen > 0)
		_data.save(ar.var("bits"));		
	return ar;
}

template<typename WordAccess>
inline OutArchive& BitArrayGeneric<WordAccess>::save(OutArchive& ar) const {
	ar.startclass("Bitvector", 1);
	save_nocls(ar);
	ar.endclass();
	return ar;
}

template<typename WordAccess>
inline std::string BitArrayGeneric<WordAccess>::to_str() const {
	assert(length() < (1UL << 16));
	std::string s;
    for (unsigned int i = 0; i < _bitlen; ++i)
		if (bit(i)) s += '1';
		else s += '0';
		return s;
}

} //namespace
