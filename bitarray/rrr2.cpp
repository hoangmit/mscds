#include "rrr2.h"
#include "bitop.h"
#include "bitstream.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

#define SAMPLE_INT 128

namespace mscds {

uint64_t RRR2Builder::getwordz(const BitArray& v, size_t idx) {
	if (idx < v.word_count()) return v.word(idx);
	else return 0;
}

uint64_t RRR2Builder::encode(uint64_t w, unsigned int k, const BitArray& combination) {
    uint64_t r = 0;
    if (k == 0) return 0;

    for (int j = 0; k > 0; ++ j)
        if ((w & (1ull << j)) != 0) {
            int hold = k > (63 - j - 1) / 2 ? 63 - j - 1 - k : k;
            
            if (63 - j == k)
                break;

            r += combination.word((63 - j - 1) * 32 + hold); //Increment r by (63 - j - 1) C k
            -- k;
        }

    return r;
}

void RRR2Builder::build(const BitArray& b, RRR2 * o) {
	assert(b.length() <= (1ULL << 50));
	o->combination = BitArrayBuilder::create(131072);

    //-------------------------Pre-compute answers for n C j----------------
    for (int n = 63; n >= 0; --n)
        for (int r = 0; r <= n / 2; ++ r) {
            o->combination.setbits((n * 32 + r) * 64, 1, 64);

            for (int j = 1; j <= r; ++ j) {
                o->combination.setbits((n * 32 + r) * 64, o->combination.word(n * 32 + r) * (n - j + 1) / j, 64); //combination[n][r] = combination[n][r] * (n - j + 1) / j
            }
        }

    o->combination.setbits(130880, 759510004936100352, 64); //63 C 29
    o->combination.setbits(130944, 860778005594247040, 64); //63 C 30
    o->combination.setbits(131008, 916312070471295232, 64); //63 C 31

    //---------------------Building tables R and S---------------------------
    uint64_t num_of_blocks = b.length() % 63 == 0 ? b.length() / 63 : b.length() / 63 + 1;
    o->R = BitArrayBuilder::create(6 * num_of_blocks);
    OBitStream SBitStream;
    uint64_t idxR = 0, idxB = 0, curr_word;
    unsigned int step, curr_popcnt, logval;
    int hold;
    o->onecnt = 0;
    
    while (idxB < b.length()) {
        if (idxB + 63 <= b.length())
            step = 63;
        else
            step = b.length() - idxB;
       
        curr_word =  b.bits(idxB, step);
        curr_popcnt = popcnt(curr_word);
        o->onecnt += curr_popcnt;
        o->R.setbits(idxR, curr_popcnt, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
        logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(o->combination.word(63 * 32 + hold)) / 0.3010299957);
        SBitStream.puts(encode(curr_word, curr_popcnt, o->combination), logval);
        idxR += 6;
        idxB += step;
    }

    //save o->S
    SBitStream.close();
	SBitStream.build(&(o->S));

    if (o->onecnt == 0) {
        o->len = b.length();
        return;
    }

    //----------------------Building sumR and posS------------------------
    unsigned int bits_per_sumR = o->onecnt == 1 ? 1 : ceil(log10(o->onecnt) / 0.3010299957);
	o->sumR = BitArrayBuilder::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

    for (idxR = 0; (idxR / (6 * SAMPLE_INT) * bits_per_sumR) < o->sumR.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            o->sumR.setbits(idxR / (6 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += o->R.bits(idxR, 6);
    }

    unsigned int bits_per_posS = ceil(log10(o->S.length()) / 0.3010299957);
	o->posS = BitArrayBuilder::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

    for (idxR = 0; (idxR / (6 * SAMPLE_INT) * bits_per_posS) < o->posS.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            o->posS.setbits(idxR / (6 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        curr_popcnt = o->R.bits(idxR, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
        logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(o->combination.word(63 * 32 + hold)) / 0.3010299957);
        sum += logval;
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

void RRR2Builder::build(const BitArray &b, OutArchive &ar) {
	ar.startclass("RRR2", 1);
	assert(b.length() <= (1ULL << 50));
	ar.var("inventory");
	BitArray combination = BitArrayBuilder::create(131072);

    //-------------------------Pre-compute answers for n C j----------------
    for (int n = 63; n >= 0; --n)
        for (int r = 0; r <= n / 2; ++ r) {
            combination.setbits((n * 32 + r) * 64, 1, 64);

            for (int j = 1; j <= r; ++ j) {
                combination.setbits((n * 32 + r) * 64, combination.word(n * 32 + r) * (n - j + 1) / j, 64); //combination[n][r] = combination[n][r] * (n - j + 1) / j
            }
        }

    combination.setbits(130880, 759510004936100352, 64); //63 C 29
    combination.setbits(130944, 860778005594247040, 64); //63 C 30
    combination.setbits(131008, 916312070471295232, 64); //63 C 31

    //---------------------Building tables R and S---------------------------
    uint64_t num_of_blocks = b.length() % 63 == 0 ? b.length() / 63 : b.length() / 63 + 1;
	BitArray R = BitArrayBuilder::create(6 * num_of_blocks);
    OBitStream SBitStream;
    uint64_t idxR = 0, idxB = 0, curr_word;
    unsigned int step, curr_popcnt, logval;
    uint64_t onecnt = 0;
    int hold;
    
    while (idxB < b.length()) {
        if (idxB + 63 <= b.length())
            step = 63;
        else
            step = b.length() - idxB;
        
        curr_word =  b.bits(idxB, step);
        curr_popcnt = popcnt(curr_word);
        onecnt += curr_popcnt;
        R.setbits(idxR, curr_popcnt, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
        logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(combination.word(63 * 32 + hold)) / 0.3010299957);
        SBitStream.puts(encode(curr_word, curr_popcnt, combination), logval);
        idxR += 6;
        idxB += step;
    }

    //save o->S
    SBitStream.close();
	BitArray S;
	SBitStream.build(&S);

    if (onecnt == 0) {
        R.save(ar.var("R"));
        S.save(ar.var("S"));
        combination.save(ar.var("combination"));
        ar.var("onecnt").save(onecnt);
        ar.var("len").save(b.length());
        ar.endclass();
        return;
    }

    //----------------------Building sumR and posS------------------------
    unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceil(log10(onecnt) / 0.3010299957);
	BitArray sumR = BitArrayBuilder::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

    for (idxR = 0; (idxR / (6 * SAMPLE_INT) * bits_per_sumR) < sumR.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            sumR.setbits(idxR / (6 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += R.bits(idxR, 6);
    }

    unsigned int bits_per_posS = ceil(log10(S.length()) / 0.3010299957);
	BitArray posS = BitArrayBuilder::create((num_of_blocks / SAMPLE_INT == 0 ? 1 : num_of_blocks / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

    for (idxR = 0; (idxR / (6 * SAMPLE_INT) * bits_per_posS) < posS.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            posS.setbits(idxR / (6 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        curr_popcnt = R.bits(idxR, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
        logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(combination.word(63 * 32 + hold)) / 0.3010299957);
        sum += logval;
    }

    R.save(ar.var("R"));
    S.save(ar.var("S"));
    posS.save(ar.var("posS"));
    sumR.save(ar.var("sumR"));
    combination.save(ar.var("combination"));
    ar.var("onecnt").save(onecnt);
    ar.var("len").save(b.length());
	ar.endclass();

	/*ar.startclass("RRR2", 1);
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

void RRR2::savep(OutArchive &ar) const {
	ar.startclass("RRR2_rankonly", 1);
	ar.var("bit_len").save(length());
    R.save(ar.var("R"));
	S.save(ar.var("S"));
    sumR.save(ar.var("sumR"));
    posS.save(ar.var("posS"));
    combination.save(ar.var("combination"));
	ar.var("onecnt").save(onecnt);
    ar.var("len").save(len);
	ar.endclass();
}

void RRR2::loadp(InpArchive &ar, BitArray &b) {
	ar.loadclass("RRR2_rankonly");
	size_t blen;
	ar.var("bit_len").load(blen);
	if (b.length() != blen) throw std::runtime_error("length mismatch");
	//uint64_t nc = ((blen + 2047) / 2048) * 2;
    R.load(ar.var("R"));
	S.load(ar.var("S"));
    sumR.load(ar.var("sumR"));
    posS.load(ar.var("posS"));
	//this->bits = b;
    combination.load(ar.var("combination"));
	ar.var("onecnt").load(onecnt);
    ar.var("len").load(len);
	ar.endclass();
}

void RRR2::save(OutArchive &ar) const {
	ar.startclass("RRR2", 1);
	ar.var("bit_len").save(length());
    R.save(ar.var("R"));
	S.save(ar.var("S"));
    sumR.save(ar.var("sumR"));
    posS.save(ar.var("posS"));
    combination.save(ar.var("combination"));
	ar.var("onecnt").save(onecnt);
    ar.var("len").save(len);
	//bits.save(ar.var("bits"));
	ar.endclass();
}

void RRR2::load(InpArchive &ar) {
	ar.loadclass("RRR2");
	size_t blen;
	ar.var("bit_len").load(blen);
    R.load(ar.var("R"));
	S.load(ar.var("S"));
    sumR.load(ar.var("sumR"));
    posS.load(ar.var("posS"));
    combination.load(ar.var("combination"));
	ar.var("onecnt").load(onecnt);
    ar.var("len").load(len);
	//bits.load(ar.var("bits"));
	ar.endclass();
	//if (bits.length() != blen) throw std::runtime_error("length mismatch");
}



bool RRR2::bit(uint64_t p) const {
    return (rank(p+1)-rank(p))==1;
	//return bits.bit(p);
}

uint64_t RRR2::partialsum(uint64_t block) const {
    unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceil(log10(onecnt) / 0.3010299957);
    uint64_t j = (block / SAMPLE_INT) < (sumR.length() / bits_per_sumR) ? (block / SAMPLE_INT) : (sumR.length() / bits_per_sumR - 1);
    uint64_t sum = sumR.bits(j * bits_per_sumR, bits_per_sumR);

    for (j = j * SAMPLE_INT; j < block; ++j)
        sum += R.bits(j * 6, 6);

    return sum;
}

uint64_t RRR2::positionS(uint64_t block) const {
    unsigned int bits_per_posS = ceil(log10(S.length()) / 0.3010299957), curr_popcnt, logval;
    uint64_t j = (block / SAMPLE_INT) < (posS.length() / bits_per_posS) ? (block / SAMPLE_INT) : (posS.length() / bits_per_posS - 1);
    uint64_t pos = posS.bits(j * bits_per_posS, bits_per_posS);

    for (j = j * SAMPLE_INT; j < block; ++j) {
        curr_popcnt = R.bits(j * 6, 6);
        int hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
        logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(combination.word(63 * 32 + hold)) / 0.3010299957);
        pos += logval;
    }

    return pos;
}

uint64_t RRR2::decode(uint64_t offset, unsigned int k) const {
    if (k == 0) return 0;
    uint64_t word = 0ull;
    unsigned int t = 63, j = 0;

    while (k > 0) {
        int hold = k > (63 - j - 1) / 2 ? 63 - j - 1 - k : k;

        if (63 - j == k) {
            word |= ~((1ull << j) - 1) & ((1ull << 63) - 1);
            break;
        }

        if (offset >= combination.word((63 - j - 1) * 32 + hold)) {
           word |= (1ull << j);
           offset -= combination.word((63 - j - 1) * 32 + hold);
           -- k;
        }

        ++ j;
    }

    return word;
}

uint64_t RRR2::rank(const uint64_t p) const {
    if (onecnt == 0)
        return 0;

    int hold;
    uint64_t block = p / 63;
    uint64_t sum = partialsum(block);
    if (block * 63 == p) return sum;
    uint64_t pos = positionS(block);
    unsigned int curr_popcnt = R.bits(block * 6, 6);
    hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
    unsigned int logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(combination.word(63 * 32 + hold)) / 0.3010299957);
    uint64_t offset = S.bits(pos, logval);
    uint64_t word = decode(offset, curr_popcnt);
    return sum + popcnt(word & ((1ull << (p % 63)) - 1));

/*	assert(p <= bits.length());
	if (p == bits.length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 10) & ~1ull;

	uint64_t val = (inv.word(blk) & 0x3FFFFFFFFFFFFULL) + subblkrank(blk, ((p >> 8) & 7ULL));
	for (unsigned int i = 0; i < (unsigned int) (wpos & 3ULL); ++i)
		val += popcnt(bits.word(i + (wpos & ~3ULL)));
	return val + word_rank(wpos, p & 63ULL);*/
}

uint64_t RRR2::rankzero(uint64_t p) const {
	return p - rank(p);
}

/*uint64_t RRR2::blkrank(size_t blk) const {
	return inv.word(2*blk) & 0x3FFFFFFFFFFFFULL;
}

uint64_t RRR2::subblkrank(size_t blk, unsigned int off) const {
	// off = [0..7]
	const unsigned int hi = inv.word(blk) >> 50;
	off = (off - 1) & 7;
	uint64_t subblk_rank = (inv.word(blk + 1) >>  off * 9) & 0x1FFULL;
	subblk_rank |= ((hi >> off * 2) & 3ULL) << 9;
	return subblk_rank;
} */

uint64_t RRR2::select(const uint64_t r) const {
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
        sum += R.bits(block * 6, 6);

        if (sum >= i)
            break;

        ++ block;
    }

    sum -= R.bits(block * 6, 6);
    int hold;
    uint64_t pos = positionS(block);
    unsigned int curr_popcnt = R.bits(block * 6, 6);
    hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
    unsigned int logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceil(log10(combination.word(63 * 32 + hold)) / 0.3010299957);
    uint64_t offset = S.bits(pos, logval);
    uint64_t word = decode(offset, curr_popcnt);
    return selectword(word, i - sum - 1)  + block * 63;
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

/*uint64_t RRR2::selectblock(uint64_t blk, uint64_t d) const {
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

uint64_t RRR2::blkrank0(size_t blk) const {
	return blk * 2048 - (inv.word(blk*2) & 0x3FFFFFFFFFFFFULL);
}

uint64_t RRR2::subblkrank0(size_t blk, unsigned int off) const {
	return off*4*64 - subblkrank(blk, off);
}*/

uint64_t RRR2::selectzero(uint64_t r) const {
	assert(r < len - onecnt);
	r += 1;
	uint64_t start = 0, end = length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rankzero(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}

/*uint64_t RRR2::selectblock0(uint64_t lo, uint64_t d) const {
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

void RRR2::clear() {
    //bits.clear();
	R.clear();
	S.clear();
    sumR.clear();
    posS.clear();
    combination.clear();
	onecnt = 0;
    len = 0;
}

/*unsigned int RRR2::word_rank(size_t idx, unsigned int i) const {
	return (i > 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}*/

//------------------------------------------------------------------------

struct BlockIntIterator {
/*	const RRR2& r;
	uint64_t p;
	BlockIntIterator(const RRR2& _r): r(_r), p(0) {}
	void operator++() { ++p; }
	uint64_t operator*() const { return r.blkrank(p); }*/
};

void RRR2HintSel::init() {
/*	hints.clear();
	BlockIntIterator it(rankst);
	// every 4096 positions
	hints = bsearch_hints(it, rankst.inv.word_count()/2, rankst.one_count(), 12); */
}

void RRR2HintSel::init(RRR2& r) {
	hints.clear();
	this->rankst = r;
	init();
}

void RRR2HintSel::init(BitArray& b) {
	hints.clear();
	//RRR2Builder bd;
	RRR2Builder::build(b, &rankst);
	init();
}

uint64_t RRR2HintSel::select(uint64_t r) const {
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

void RRR2HintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_
