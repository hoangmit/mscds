#include "rrr3.h"

namespace mscds {

RRR_WordAccessBuilder::RRR_WordAccessBuilder() { init(); }

void RRR_WordAccessBuilder::init() {
    i = 0;
    j = 0;
    overflow_mark = 0;
}

void RRR_WordAccessBuilder::add(uint64_t word) {
    unsigned k = popcnt(word);
    uint64_t ofs = codec.encode(64, k, word);
    unsigned len = codec.offset_len(64, k);
	offset.puts(ofs, len);
    bitcnt.puts(k & 63, 6);

    if (k == 64) overflow_mark |= (1u << j);
    ++j;

    ++i;

    if (i % RRR_WordAccess::OVERFLOW_BLK == 0) {
        _flush_overflow();
    }
    if (i % RRR_WordAccess::OFFSET_BLK == 0) {
        opos.add(offset.length());
    }
}

void RRR_WordAccessBuilder::_flush_overflow() {
	if (overflow_mark == 0) {
		pmark.put0();
	} else {
		pmark.put1();
		overflow.puts(overflow_mark, RRR_WordAccess::OVERFLOW_BLK);
	}
	overflow_mark = 0;
	j = 0;
}

void RRR_WordAccessBuilder::build(RRR_WordAccess *out) {
    offset.build(&out->offset);
    bitcnt.build(&out->bitcnt);
    if (j > 0)  _flush_overflow();
    BitArray bax;
    pmark.build(&bax);
    Rank25pBuilder::build(bax, &out->pmark);
    overflow.build(&out->overflow);
	opos.build(&out->opos);
    init();
}

void RRR_WordAccessBuilder::build_array(const BitArray &ba, RRR_WordAccess *out) {
    unsigned wc = ba.word_count();
    RRR_WordAccessBuilder bd;
    for (unsigned i = 0; i < wc; ++i) {
        bd.add(ba.word(i));
    }
    bd.build(out);
}

//--------------------------------------------------------


uint64_t RRR_WordAccess::word(size_t i) const {
    uint8_t k = popcntw(i);
    unsigned len = codec.offset_len(64, k);
    uint64_t start = offset_loc(i);
    uint64_t codev = offset.bits(start, len);
    return codec.decode(64, k, codev);
}

uint8_t RRR_WordAccess::popcntw(size_t i) const {
    uint8_t l = bitcnt.bits(i*6, 6);
    if (l > 0 || !pmark.bit(i/OVERFLOW_BLK)) return l;
    unsigned r = pmark.rank(i/OVERFLOW_BLK);
    if (overflow.bit(r * OVERFLOW_BLK + i % OVERFLOW_BLK)) return 64;
    else return 0;
}

uint64_t RRR_WordAccess::offset_loc(unsigned i) const {
    size_t ix = i / OFFSET_BLK;
    uint64_t _ops = ix == 0 ? 0 : opos[ix-1];
    unsigned id = (i % OFFSET_BLK);
    size_t ip = i - id;
    unsigned px = 0;
    for (unsigned j = 0; j < id; ++j)
        px += offset_len(ip+j);
    return px + _ops;
}

uint8_t RRR_WordAccess::offset_len(unsigned i) const {
    // 0 and 64 have the same code length
    return codec.offset_len(64, bitcnt.bits(i*6, 6));
}

void mscds::RRR_WordAccess::load(mscds::InpArchive &ar) {
	ar.loadclass("RRR_WordAccess");
	bitcnt.load(ar.var("bitcnt"));
	offset.load(ar.var("offset"));
	pmark.load(ar.var("overflow_mark"));
	overflow.load(ar.var("overflow"));
	opos.load(ar.var("offset_sparse_positions"));
	ar.endclass();
}

void RRR_WordAccess::save(OutArchive &ar) const {
	ar.startclass("RRR_WordAccess");
	bitcnt.save(ar.var("bitcnt"));
	offset.save(ar.var("offset"));
	pmark.save(ar.var("overflow_mark"));
	overflow.save(ar.var("overflow"));
	opos.save(ar.var("offset_sparse_positions"));
	ar.endclass();
}

size_t RRR_WordAccess::word_count() const { return bitcnt.length() / 6; }

void RRR_WordAccess::clear() {
    opos.clear();
    offset.clear();
    bitcnt.clear();
    pmark.clear();
    overflow.clear();
}

AdaptiveWordAccesss::AdaptiveWordAccesss() { cache.clear(); }

uint64_t AdaptiveWordAccesss::word(size_t i) const {
    if (mark.bit(i)) {
        unsigned p = mark.rank(i);
        return cache.get(p, [this, p](size_t i) {return this->rrr.word(i); });
    } else {
        unsigned p = mark.rankzero(i);
        return raw.word(p);
    }
}

uint8_t AdaptiveWordAccesss::popcntw(size_t i) const {
    if (mark.bit(i)) {
        unsigned p = mark.rank(i);
        return rrr.popcntw(p);
    } else {
        unsigned p = mark.rankzero(i);
        return popcnt(raw.word(p));
    }
}

void AdaptiveWordAccesss::load(InpArchive &ar) {
    ar.loadclass("adaptive_rrr");
    mark.load(ar.var("mark"));
    raw.load(ar.var("raw"));
    rrr.load(ar.var("rrr"));
    ar.endclass();
    cache.clear();
}

void AdaptiveWordAccesss::save(OutArchive &ar) const {
    ar.startclass("adaptive_rrr");
    mark.save(ar.var("mark"));
    raw.save(ar.var("raw"));
    rrr.save(ar.var("rrr"));
    ar.endclass();
}

void AdaptiveWordAccesss::clear() {
    mark.clear();
    raw.clear();
    rrr.clear();
    cache.clear();
}

size_t AdaptiveWordAccesss::word_count() const {
    return rrr.word_count() + raw.word_count();
}

void AdaptiveWordAccesssBuilder::add(uint64_t word) {
    unsigned k = popcnt(word);
    //break even at <24 and >40
    if (k < 24 || k > 40) {
        mark.put1();
        rrr.add(word);
    } else {
        mark.put0();
        raw.puts(word);
    }
}

void AdaptiveWordAccesssBuilder::build(AdaptiveWordAccesss *out) {
    BitArray _mark;
    mark.build(&_mark);
    Rank25pBuilder::build(_mark, &out->mark);
    raw.build(&out->raw);
    rrr.build(&out->rrr);
}

void AdaptiveWordAccesssBuilder::build_array(const BitArray &ba, AdaptiveWordAccesss *out) {
    unsigned wc = ba.word_count();
    AdaptiveWordAccesssBuilder bd;
    for (unsigned i = 0; i < wc; ++i) {
        bd.add(ba.word(i));
    }
    bd.build(out);
}

}//namespace

