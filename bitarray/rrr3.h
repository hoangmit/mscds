#pragma once

/**
\file
Another variant of RRR Compressed Rank/Select data structure.
*/

#include "codec/rrr_codec.h"
#include "bitarray.h"
#include "rank25p.h"
#include "rank6p.h"

#include "bitstream.h"


namespace mscds {
class RRR_WordAccessBuilder;

class RRR_WordAccess: public WordAccessInterface {
public:
	uint64_t word(size_t i) const;
	void setword(size_t i, uint64_t v) { throw std::runtime_error("not supported"); }
	uint8_t getchar(size_t i) const {
		uint64_t _word = this->word(i / 8);
		return (uint8_t)((_word >> (8*(i % 8))) & 0xFF);
	}
	uint8_t popcntw(size_t i) const;
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	size_t word_count() const;
	void clear();
	typedef RRR_WordAccessBuilder BuilderTp;
private:
	uint64_t offset_loc(unsigned i) const;
	uint8_t offset_len(unsigned i) const;

	friend class RRR_WordAccessBuilder;
private:
	friend class RRR_WordAccessBuilder;
	static const unsigned OFFSET_BLK = 32;
	coder::RRR_Codec codec;
	
	FixedWArray opos;
	BitArray offset;

	static const unsigned OVERFLOW_BLK = 16;
	BitArray bitcnt;
	Rank25p pmark;
	BitArray overflow;
};

template<unsigned int CACHE_SIZE = 8>
class WordCache {
public:
	WordCache() { clear(); }
	void clear() {
		for (unsigned i = 0; i < CACHE_SIZE; ++i) cache[i].first = ~0ull;
	}

	template<typename Func>
	uint64_t get(size_t i, Func fx) const {
		Entry & px = cache[i % CACHE_SIZE];
		if (px.first == i) {
			return px.second;
		} else {
			px.first = i;
			uint64_t val = fx(i);
			px.second = val;
			return val;
		}
	}
	typedef std::pair<size_t, uint64_t> Entry;
	mutable Entry cache[CACHE_SIZE];
};

class RRR_WordAccessBuilder {
public:
	RRR_WordAccessBuilder();
	void init();
	void add(uint64_t word);
	void build(RRR_WordAccess* out);
	static void build_array(const BitArray& ba, RRR_WordAccess* out);
private:
	void _flush_overflow();
private:
	OBitStream offset, bitcnt, pmark, overflow;
	coder::RRR_Codec codec;
	unsigned overflow_mark;

	FixedWArrayBuilder opos;

	unsigned i, j;
};
//-------------------------------------

class AdaptiveWordAccesssBuilder;

class AdaptiveWordAccesss: public WordAccessInterface {
public:
	AdaptiveWordAccesss();
	uint64_t word(size_t i) const;
	void setword(size_t i, uint64_t v) { throw std::runtime_error("not supported"); }
	uint8_t getchar(size_t i) const {
		uint64_t _word = this->word(i / 8);
		return (uint8_t)((_word >> (8*(i % 8))) & 0xFF);
	}

	uint8_t popcntw(size_t i) const;

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	void clear();
	size_t word_count() const;
	typedef AdaptiveWordAccesssBuilder BuilderTp;
private:
	friend class AdaptiveWordAccesssBuilder;
	Rank25p mark;
	BitArray raw;
	RRR_WordAccess rrr;
	WordCache<8> cache;
};


class AdaptiveWordAccesssBuilder {
public:
	void add(uint64_t word);
	void build(AdaptiveWordAccesss* out);
	static void build_array(const BitArray& ba, AdaptiveWordAccesss* out);
private:
	OBitStream mark, raw;
	RRR_WordAccessBuilder rrr;
};

//--------------------------------------

class AdaptiveExtWordAccesssBuilder;

class AdaptiveExtWordAccesss: public WordAccessInterface {
public:
	uint64_t word(size_t i) const;
	void setword(size_t i, uint64_t v);
	uint8_t getchar(size_t i) const;
	uint8_t popcntw(size_t i) const;
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	size_t word_count() const;
	typedef AdaptiveExtWordAccesssBuilder BuilderTp;
private:
	friend class AdaptiveExtWordAccesssBuilder;
	uint8_t rrr_popcntw(size_t i) const;
	uint8_t offset_len(unsigned i) const; 
	uint64_t rrr_word(size_t i) const;
	uint64_t offset_loc(unsigned i) const;
	static const unsigned OFFSET_BLK = 32;
private:
	Rank25p mark;
	BitArray raw;
	BitArray bitcnt;
	BitArray offset;

	FixedWArray opos;
	coder::RRR_Codec codec;
	WordCache<4> cache;
};

class AdaptiveExtWordAccesssBuilder {
public:
	AdaptiveExtWordAccesssBuilder(): cc(0) {}
	void add(uint64_t word);
	void build(AdaptiveExtWordAccesss* out);
	static void build_array(const BitArray& ba, AdaptiveExtWordAccesss* out);
private:
	OBitStream mark, raw, offset, bitcnt;
	FixedWArrayBuilder opos;
	coder::RRR_Codec codec;
	size_t cc;
};

//--------------------------------------

template<typename WordAccess>
class Word_BitArrayBuilder;

template<typename WordAccess>
struct Word_BitArray: public BitArrayGeneric<WordAccess> {
	typedef Word_BitArrayBuilder<WordAccess> BuilderTp;
	friend class Word_BitArrayBuilder<WordAccess>;
	typedef BitArrayGeneric<WordAccess> ParentTp;
	void clear() {
		ParentTp::_bitlen = 0;
		ParentTp::_data.clear();
	}
	void load(InpArchive& ar) {
		ar.loadclass("Generic_Word_BitArray");
		ar.var("length").load(ParentTp::_bitlen);
		ParentTp::_data.load(ar.var("bit_data"));
		ar.endclass();
	}
	void save(OutArchive& ar) const {
		ar.startclass("Generic_Word_BitArray");
		ar.var("length").save(ParentTp::_bitlen);
		ParentTp::_data.save(ar.var("bit_data"));
		ar.endclass();
	}
};

template<typename WordAccess>
class Word_BitArrayBuilder {
public:
	static void build_array(const BitArray& ba, Word_BitArray<WordAccess>* out) {
		typedef typename WordAccess::BuilderTp BuilderTp;
		BuilderTp::build_array(ba, &(out->_data));
		out->_bitlen = ba.length();
	}
private:
};

typedef Word_BitArray<AdaptiveWordAccesss> RRR_BitArray;
typedef Word_BitArrayBuilder<AdaptiveWordAccesss> RRR_BitArrayBuilder;

class RRR3_RankBuilder;

/// Rank for RRR3
class RRR3_Rank: public Rank6pAux {
public:
	void clear();
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	typedef RRR3_RankBuilder BuilderTp;
protected:
	friend class RRR3_RankBuilder;
	RRR_BitArray rrr_bits;
};


class RRR3_RankBuilder {
public:
	static void build(const BitArray& b, RRR3_Rank * o) {
		RRR_BitArrayBuilder::build_array(b, &o->rrr_bits);
		Rank6pBuilder::build_aux(&o->rrr_bits, o);
	}
};

}//namespace
