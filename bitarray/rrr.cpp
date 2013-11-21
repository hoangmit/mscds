#include "rrr.h"
#include "bitop.h"
#include "bitstream.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

#define SAMPLE_INT 100

using namespace std;
namespace mscds {

uint64_t RRRBuilder::getwordz(const BitArray& v, size_t idx) {
	if (idx < v.word_count()) return v.word(idx);
	else return 0;
}

void RRRBuilder::build(const BitArray& b, RRR * o) {
	assert(b.length() <= (1ULL << 50));

    //--------------------Building table E-----------------------------
    o->E = BitArray::create(524288); //16 * 2^15
    uint64_t idxE = 0, curr_offset;
    unsigned int offset[32768];
    bool selector[15];

    for (int r = 0; r <= 15; ++ r) {
        for (int i=0; i<15-r; ++i)
            selector[i]=false;

        for (int i=15-r; i<15; ++i)
            selector[i]=true;

        curr_offset = 0;

        do {
            uint64_t curr_comb = 0;

            for (int i = 0; i < 15; ++ i)
                if (selector[14 - i])
                    curr_comb |= (1ull << i);
           
            o->E.setbits(idxE, curr_comb, 16);
            idxE += 16;

            offset[curr_comb] = curr_offset++;
        } while (std::next_permutation(selector, selector+15));
    }

    //---------------------Building tables R and S---------------------------
    unsigned int logtable[16] = {1, 4, 7, 9, 11, 12, 13, 13, 13, 13, 12, 11, 9, 7, 4, 1}; //ceil(log2(15 C i))
    uint64_t num_of_blocks = b.length() % 15 == 0 ? b.length() / 15 : b.length() / 15 + 1;
    o->R = BitArray::create(4 * num_of_blocks);
    OBitStream SBitStream;
    uint64_t idxR = 0, idxB = 0;
    unsigned int step, curr_word, curr_popcnt;
    o->onecnt = 0;
    
    while (idxB < b.length()) {
        if (idxB + 15 <= b.length())
            step = 15;
        else
            step = b.length() - idxB;
        
        curr_word = b.bits(idxB, step);
        curr_popcnt = popcnt(curr_word);
        o->onecnt += curr_popcnt;
        o->R.setbits(idxR, curr_popcnt, 4);
        SBitStream.puts(offset[curr_word], logtable[curr_popcnt]);
        idxR += 4;
        idxB += step;
    }

    //save o->S
    SBitStream.close();
    o->S = BitArray::create(SBitStream.data_ptr(), SBitStream.length());

    if (o->onecnt == 0)
        return;

    //----------------------Building sumR and posS------------------------
    unsigned int bits_per_sumR = o->onecnt == 1 ? 1 : ceil(log10(o->onecnt) / 0.3010299957);
    o->sumR = BitArray::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

    for (idxR = 0; (idxR / (4 * SAMPLE_INT) * bits_per_sumR) < o->sumR.length(); idxR += 4) {
        if (idxR % (4 * SAMPLE_INT) == 0)
            o->sumR.setbits(idxR / (4 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += o->R.bits(idxR, 4);
    }

    unsigned int bits_per_posS = ceil(log10(o->S.length()) / 0.3010299957);
    o->posS = BitArray::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

    for (idxR = 0; (idxR / (4 * SAMPLE_INT) * bits_per_posS) < o->posS.length(); idxR += 4) {
        if (idxR % (4 * SAMPLE_INT) == 0)
            o->posS.setbits(idxR / (4 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        sum += logtable[o->R.bits(idxR, 4)];
    }
    o->len=b.length();

/*	assert(b.length() <= (1ULL << 50));
	o->bits = b;
	uint64_t nc = ((o->bits.length() + 2047) / 2048) * 2;
	o->inv = BitArray::create(nc*64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits.word_count()) {
		o->inv.word(pos) = cnt;
		for (unsigned int k = 0; k < 4; k++)
			cnt += popcnt(getwordz(o->bits, i+k));

		uint64_t overflow = 0;
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t val = cnt - o->inv.word(pos);
			o->inv.word(pos + 1) |= (val & 0x1FF) << 9 * (j - 1);
			overflow |= ((val >> 9) & 3) << 2 * (j - 1);
			for (unsigned int k = 0; k < 4; k++)
				cnt += popcnt(getwordz(o->bits, i + j * 4 + k));
		}
		o->inv.word(pos) |= overflow << 50;
		i += 8*4; pos += 2;
	}
	o->onecnt = cnt;*/
}

void RRRBuilder::build(const BitArray &b, OArchive &ar) {
	ar.startclass("RRR", 1);
	assert(b.length() <= (1ULL << 50));
	ar.var("inventory");

    //--------------------Building table E-----------------------------
    BitArray E = BitArray::create(524288); //16 * 2^15
    uint64_t idxE = 0, curr_offset;
    unsigned int offset[32768];
    bool selector[15];

    for (int r = 0; r <= 15; ++ r) {
        for (int i=0; i<15-r; ++i)
            selector[i]=false;

        for (int i=15-r; i<15; ++i)
            selector[i]=true;

        curr_offset = 0;

        do {
            uint64_t curr_comb = 0;

            for (int i = 0; i < 15; ++ i)
                if (selector[14 - i])
                    curr_comb |= (1ull << i);
           
            E.setbits(idxE, curr_comb, 16);
            idxE += 16;

            offset[curr_comb] = curr_offset++;
        } while (std::next_permutation(selector, selector+15));
    }

    //---------------------Building tables R and S---------------------------
    unsigned int logtable[16] = {1, 4, 7, 9, 11, 12, 13, 13, 13, 13, 12, 11, 9, 7, 4, 1}; //ceil(log2(15 C i))
    uint64_t num_of_blocks = b.length() % 15 == 0 ? b.length() / 15 : b.length() / 15 + 1;
    BitArray R = BitArray::create(4 * num_of_blocks);
    OBitStream SBitStream;
    uint64_t idxR = 0, idxB = 0;
    unsigned int step, curr_word, curr_popcnt;
    uint64_t onecnt = 0;
    
    while (idxB < b.length()) {
        if (idxB + 15 <= b.length())
            step = 15;
        else
            step = b.length() - idxB;
        
        curr_word = b.bits(idxB, step);
        curr_popcnt = popcnt(curr_word);
        onecnt += curr_popcnt;
        R.setbits(idxR, curr_popcnt, 4);
        SBitStream.puts(offset[curr_word], logtable[curr_popcnt]);
        idxR += 4;
        idxB += step;
    }

    //save o->S
    SBitStream.close();
    BitArray S = BitArray::create(SBitStream.data_ptr(), SBitStream.length());

    if (onecnt == 0)
        return;

    //----------------------Building sumR and posS------------------------
    unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceil(log10(onecnt) / 0.3010299957);
    BitArray sumR = BitArray::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

    for (idxR = 0; (idxR / (4 * SAMPLE_INT) * bits_per_sumR) < sumR.length(); idxR += 4) {
        if (idxR % (4 * SAMPLE_INT) == 0)
            sumR.setbits(idxR / (4 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += R.bits(idxR, 4);
    }

    unsigned int bits_per_posS = ceil(log10(S.length()) / 0.3010299957);
    BitArray posS = BitArray::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

    for (idxR = 0; (idxR / (4 * SAMPLE_INT) * bits_per_posS) < posS.length(); idxR += 4) {
        if (idxR % (4 * SAMPLE_INT) == 0)
            posS.setbits(idxR / (4 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        sum += logtable[R.bits(idxR, 4)];
    }

    E.save(ar.var("E"));
    R.save(ar.var("R"));
    S.save(ar.var("S"));
    posS.save(ar.var("posS"));
    sumR.save(ar.var("sumR"));
    ar.var("onecnt").save(onecnt);
    ar.var("len").save(b.length());
	ar.endclass();

	/*ar.startclass("RRR", 1);
	assert(b.length() <= (1ULL << 50));
	ar.var("bit_len").save(b.length());

	uint64_t nc = ((b.length() + 2047) / 2048) * 2; // every 2048 bits is a block
	ar.var("inventory");
	BitArraySeqBuilder bd(nc, ar);

	uint64_t w1 = 0, w2 = 0;

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < b.word_count()) {
		w1 = cnt;
		for (unsigned int k = 0; k < 4; k++)
			cnt += popcnt(getwordz(b, i+k));

		uint64_t overflow = 0;
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t val = cnt - w1;
			w2 |= (val & 0x1FF) << 9 * (j - 1);
			overflow |= ((val >> 9) & 3) << 2 * (j - 1);
			for (unsigned int k = 0; k < 4; k++)
				cnt += popcnt(getwordz(b, i + j * 4 + k));
		}
		w1 |= overflow << 50;
		i += 8*4; pos += 2;
		bd.addword(w1);
		bd.addword(w2);
		w1 = 0; w2 = 0;
	}
	bd.done();
	ar.var("onecnt").save(cnt);
	ar.endclass();*/
}

void RRR::savep(OArchive &ar) const {
	ar.startclass("RRR_rankonly", 1);
	ar.var("bit_len").save(length());
	E.save(ar.var("E"));
    R.save(ar.var("R"));
	S.save(ar.var("S"));
    sumR.save(ar.var("sumR"));
    posS.save(ar.var("posS"));
	ar.var("onecnt").save(onecnt);
    ar.var("len").save(len);
	ar.endclass();
}

void RRR::loadp(IArchive &ar, BitArray &b) {
	ar.loadclass("RRR_rankonly");
	size_t blen;
	ar.var("bit_len").load(blen);
	if (b.length() != blen) throw std::runtime_error("length mismatch");
	//uint64_t nc = ((blen + 2047) / 2048) * 2;
	E.load(ar.var("E"));
    R.load(ar.var("R"));
	S.load(ar.var("S"));
    sumR.load(ar.var("sumR"));
    posS.load(ar.var("posS"));
	//this->bits = b;
	ar.var("onecnt").load(onecnt);
    ar.var("len").load(len);
	ar.endclass();
}

void RRR::save(OArchive &ar) const {
	ar.startclass("RRR", 1);
	ar.var("bit_len").save(length());
	E.save(ar.var("E"));
    R.save(ar.var("R"));
	S.save(ar.var("S"));
    sumR.save(ar.var("sumR"));
    posS.save(ar.var("posS"));
	ar.var("onecnt").save(onecnt);
    ar.var("len").save(len);
	//bits.save(ar.var("bits"));
	ar.endclass();
}

void RRR::load(IArchive &ar) {
	ar.loadclass("RRR");
	size_t blen;
	ar.var("bit_len").load(blen);
	E.load(ar.var("E"));
    R.load(ar.var("R"));
	S.load(ar.var("S"));
    sumR.load(ar.var("sumR"));
    posS.load(ar.var("posS"));
	ar.var("onecnt").load(onecnt);
    ar.var("len").load(len);
	//bits.load(ar.var("bits"));
	ar.endclass();
	//if (bits.length() != blen) throw std::runtime_error("length mismatch");
}



bool RRR::bit(uint64_t p) const {
    return (rank(p+1)-rank(p))==1;
	//return bits.bit(p);
}

uint64_t RRR::partialsum(uint64_t block) const {
    unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceil(log10(onecnt) / 0.3010299957);
    uint64_t j = (block / SAMPLE_INT) < (sumR.length() / bits_per_sumR) ? (block / SAMPLE_INT) : (sumR.length() / bits_per_sumR - 1);
    uint64_t sum = sumR.bits(j * bits_per_sumR, bits_per_sumR);

    for (j = j * SAMPLE_INT; j < block; ++j)
        sum += R.bits(j * 4, 4);

    return sum;
}

uint64_t RRR::positionS(uint64_t block) const {
    unsigned int logtable[16] = {1, 4, 7, 9, 11, 12, 13, 13, 13, 13, 12, 11, 9, 7, 4, 1}; //ceil(log2(15 C i))
    unsigned int bits_per_posS = ceil(log10(S.length()) / 0.3010299957);
    uint64_t j = (block / SAMPLE_INT) < (posS.length() / bits_per_posS) ? (block / SAMPLE_INT) : (posS.length() / bits_per_posS - 1);
    uint64_t pos = posS.bits(j * bits_per_posS, bits_per_posS);

    for (j = j * SAMPLE_INT; j < block; ++j)
        pos += logtable[R.bits(j * 4, 4)];

    return pos;
}

uint64_t RRR::rank(const uint64_t p) const {
    if (onecnt == 0)
        return 0;

    uint64_t block = p / 15;
    uint64_t sum = partialsum(block);
    if (block * 15 == p) return sum;
    uint64_t pos = positionS(block);
    unsigned int logtable[16] = {1, 4, 7, 9, 11, 12, 13, 13, 13, 13, 12, 11, 9, 7, 4, 1}; //ceil(log2(15 C i))
    unsigned int Elength[16] = {1, 16, 121, 576, 1941, 4944, 9949, 16384, 22819, 27824, 30827, 32192, 32647, 32752, 32767, 32768}; //cumulative sum of (15 C i)
    unsigned int blockcount = R.bits(block * 4, 4);
    unsigned int offset = S.bits(pos, logtable[blockcount]);
    unsigned int word = blockcount == 0 ? 0 : E.bits((Elength[blockcount - 1] + offset) * 16, 16);

    return sum + popcnt(word & ((1 << (p % 15)) - 1));

/*	assert(p <= bits.length());
	if (p == bits.length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 10) & ~1ull;

	uint64_t val = (inv.word(blk) & 0x3FFFFFFFFFFFFULL) + subblkrank(blk, ((p >> 8) & 7ULL));
	for (unsigned int i = 0; i < (unsigned int) (wpos & 3ULL); ++i)
		val += popcnt(bits.word(i + (wpos & ~3ULL)));
	return val + word_rank(wpos, p & 63ULL);*/
}

uint64_t RRR::rankzero(uint64_t p) const {
	return p - rank(p);
}

/*uint64_t RRR::blkrank(size_t blk) const {
	return inv.word(2*blk) & 0x3FFFFFFFFFFFFULL;
}

uint64_t RRR::subblkrank(size_t blk, unsigned int off) const {
	// off = [0..7]
	const unsigned int hi = inv.word(blk) >> 50;
	off = (off - 1) & 7;
	uint64_t subblk_rank = (inv.word(blk + 1) >>  off * 9) & 0x1FFULL;
	subblk_rank |= ((hi >> off * 2) & 3ULL) << 9;
	return subblk_rank;
} */

uint64_t RRR::select(const uint64_t r) const {
	assert(r < onecnt); //onecnt cannot be 0
    uint64_t i = r + 1;
    unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceil(log10(onecnt) / 0.3010299957);
    uint64_t start = 0, end = sumR.length() / bits_per_sumR - 1, avg;
    uint64_t sum, sum2;

    while (start <= end) {
        avg = (start + end) / 2;
        sum = sumR.bits(avg * bits_per_sumR, bits_per_sumR);
        sum2 = avg + 1 < sumR.length() / bits_per_sumR ? sumR.bits((avg + 1) * bits_per_sumR, bits_per_sumR) : sum;

        if (sum < i && sum2 >= i)
            break;

        if (sum >= i)
            end = avg - 1;
        else
            start = avg + 1;
    }
    
    //Now search in table R
    uint64_t block = avg * SAMPLE_INT;

    while (true) {
        sum += R.bits(block * 4, 4);

        if (sum >= i)
            break;

        ++ block;
    }

    sum -= R.bits(block * 4, 4);

    uint64_t pos = positionS(block);
    unsigned int logtable[16] = {0, 4, 7, 9, 11, 12, 13, 13, 13, 13, 12, 11, 9, 7, 4, 0}; //ceil(log2(15 C i))
    unsigned int Elength[16] = {1, 16, 121, 576, 1941, 4944, 9949, 16384, 22819, 27824, 30827, 32192, 32647, 32752, 32767, 32768}; //cumulative sum of (15 C i)
    unsigned int blockcount = R.bits(block * 4, 4);
    unsigned int offset = S.bits(pos, logtable[blockcount]);
    unsigned int word = blockcount == 0 ? 0 : E.bits((Elength[blockcount - 1] + offset) * 16, 16);
    return selectword(word, i - sum - 1)  + block * 15;
/*	assert(r < onecnt);
	uint64_t lo = 0, len = inv.word_count() / 2;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (blkrank(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else
			len = d;
	}
	if (lo >= inv.word_count() / 2 || r < blkrank(lo))
		lo -= 1;
	return selectblock(lo, r - blkrank(lo));*/
}

/*uint64_t RRR::selectblock(uint64_t blk, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank(blk*2, i) <= d) j = i;
		else break;
	d = d - subblkrank(blk*2, j);
	uint64_t widx = blk * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr = popcnt(bits.word(widx));
		if (d < wr)
			return blk * 2048 + 256* j + 64 * k + selectword(bits.word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	return ~0ull;
}

uint64_t RRR::blkrank0(size_t blk) const {
	return blk * 2048 - (inv.word(blk*2) & 0x3FFFFFFFFFFFFULL);
}

uint64_t RRR::subblkrank0(size_t blk, unsigned int off) const {
	return off*4*64 - subblkrank(blk, off);
}*/

uint64_t RRR::selectzero(const uint64_t r) const {
	assert(r < len - onecnt);
    uint64_t start = 1, end = len, avg;

    while (start <= end) {
        avg = (start + end) / 2;

        if (rankzero(avg) < r + 1)
            start = avg + 1;
        else if (avg > 1 && rankzero(avg - 1) >= r + 1)
            end = avg - 1;
        else
            break;
    }

    return avg - 1;
/*	assert(r < bits.length() - onecnt);
	uint64_t lo = 0, len = inv.word_count() / 2;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (blkrank0(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else 
			len = d;
	}
	if (lo >= inv.word_count() / 2 || r < blkrank0(lo))
		lo -= 1;
	return selectblock0(lo, r - blkrank0(lo));*/
}

/*uint64_t RRR::selectblock0(uint64_t lo, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank0(lo*2, i) <= d) j = i;
		else break;
	d = d - subblkrank0(lo*2, j);
	uint64_t widx = lo * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr =  popcnt(~bits.word(widx));
		if (d < wr)
			return lo * 2048 + 256* j + 64 * k + selectword(~bits.word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	return ~0ull;
}*/

void RRR::clear() {
    //bits.clear();
    E.clear();
	R.clear();
	S.clear();
    sumR.clear();
    posS.clear();
	onecnt = 0;
    len = 0;
}

/*unsigned int RRR::word_rank(size_t idx, unsigned int i) const {
	return (i > 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}*/

//------------------------------------------------------------------------

struct BlockIntIterator {
/*	const RRR& r;
	uint64_t p;
	BlockIntIterator(const RRR& _r): r(_r), p(0) {}
	void operator++() { ++p; }
	uint64_t operator*() const { return r.blkrank(p); }*/
};

void RRRHintSel::init() {
/*	hints.clear();
	BlockIntIterator it(rankst);
	// every 4096 positions
	hints = bsearch_hints(it, rankst.inv.word_count()/2, rankst.one_count(), 12); */
}

void RRRHintSel::init(RRR& r) {
	hints.clear();
	this->rankst = r;
	init();
}

void RRRHintSel::init(BitArray& b) {
	hints.clear();
	//RRRBuilder bd;
	RRRBuilder::build(b, &rankst);
	init();
}

uint64_t RRRHintSel::select(uint64_t r) const {
    return 0;
/*	assert(r < rankst.one_count());
	uint64_t lo = hints[r >> 12]; //  % 4096
	uint64_t len = hints[(r >> 12) + 1] - lo;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (rankst.blkrank(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else
			len = d;
	}
	if (lo >= rankst.inv.word_count() / 2 || r < rankst.blkrank(lo))
		lo--;
	return rankst.selectblock(lo, r - rankst.blkrank(lo));*/
}

void RRRHintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_
