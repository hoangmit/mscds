#pragma once

#include "codec/rrr_codec.h"
#include "bitarray.h"
#include "rank25p.h"

#include "bitstream.h"

namespace mscds {
class RRR_WordAccessBuilder;

class RRR_WordAccess {
public:
    uint64_t getword(size_t i) const;
    uint8_t popcntw(size_t i) const;
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
    size_t word_count() const;
    void clear();
	typedef RRR_WordAccessBuilder BuilderTp;
private:
    uint64_t offset_loc(unsigned i) const;

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

class WordCache {
public:
	WordCache() { clear(); }

	void clear() {
		for (unsigned i = 0; i < 4; ++i) cache[i].first = ~0ull;
	}

	template<typename Func>
	uint64_t get(size_t i, Func fx) const {
		Entry & px = cache[i & 3];
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
	mutable Entry cache[4];
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


class AdaptiveWordAccesssBuilder;

class AdaptiveWordAccesss {
public:
    AdaptiveWordAccesss();
    uint64_t getword(size_t i) const;

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
	WordCache cache;
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

}//namespace
