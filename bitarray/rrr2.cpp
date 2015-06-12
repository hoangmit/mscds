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
    uint64_t num_of_blocks = (b.length() + 62) / 63;
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
		logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(o->combination.word(63 * 32 + hold));
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
	unsigned int bits_per_sumR = o->onecnt == 1 ? 1 : ceillog2(o->onecnt);
	o->sumR = BitArrayBuilder::create(((num_of_blocks + SAMPLE_INT - 1) / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

    for (idxR = 0; idxR + 6 <= o->R.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            o->sumR.setbits(idxR / (6 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += o->R.bits(idxR, 6);
    }

	unsigned int bits_per_posS = ceillog2(o->S.length());
	o->posS = BitArrayBuilder::create(((num_of_blocks + SAMPLE_INT - 1) / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

	for (idxR = 0; idxR + 6 <= o->R.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            o->posS.setbits(idxR / (6 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        curr_popcnt = o->R.bits(idxR, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
		logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(o->combination.word(63 * 32 + hold));
        sum += logval;
    }
    o->len=b.length();
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
    uint64_t num_of_blocks = (b.length() % 63) == 0 ? b.length() / 63 : b.length() / 63 + 1;
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
		logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(combination.word(63 * 32 + hold));
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
	unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceillog2(onecnt);
	BitArray sumR = BitArrayBuilder::create(((num_of_blocks + SAMPLE_INT - 1) / SAMPLE_INT) * bits_per_sumR);
    uint64_t sum = 0;

	for (idxR = 0; idxR + 6 <= R.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            sumR.setbits(idxR / (6 * SAMPLE_INT) * bits_per_sumR, sum, bits_per_sumR);

        sum += R.bits(idxR, 6);
    }

	unsigned int bits_per_posS = ceillog2(S.length());
	BitArray posS = BitArrayBuilder::create(((num_of_blocks + SAMPLE_INT - 1) / SAMPLE_INT) * bits_per_posS);
    sum = idxB = 0;

	for (idxR = 0; idxR + 6 <= R.length(); idxR += 6) {
        if (idxR % (6 * SAMPLE_INT) == 0)
            posS.setbits(idxR / (6 * SAMPLE_INT) * bits_per_posS, sum, bits_per_posS);

        curr_popcnt = R.bits(idxR, 6);
        hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
		logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(combination.word(63 * 32 + hold));
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
	unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceillog2(onecnt);
    uint64_t j = (block / SAMPLE_INT) < (sumR.length() / bits_per_sumR) ? (block / SAMPLE_INT) : (sumR.length() / bits_per_sumR - 1);
    uint64_t sum = sumR.bits(j * bits_per_sumR, bits_per_sumR);

    for (j = j * SAMPLE_INT; j < block; ++j)
        sum += R.bits(j * 6, 6);

    return sum;
}

uint64_t RRR2::positionS(uint64_t block) const {
	unsigned int bits_per_posS = ceillog2(S.length()), curr_popcnt, logval;
    uint64_t j = (block / SAMPLE_INT) < (posS.length() / bits_per_posS) ? (block / SAMPLE_INT) : (posS.length() / bits_per_posS - 1);
    uint64_t pos = posS.bits(j * bits_per_posS, bits_per_posS);

    for (j = j * SAMPLE_INT; j < block; ++j) {
        curr_popcnt = R.bits(j * 6, 6);
        int hold = curr_popcnt > 31 ? 63 - curr_popcnt : curr_popcnt;
		logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(combination.word(63 * 32 + hold));
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
	unsigned int logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(combination.word(63 * 32 + hold));
    uint64_t offset = S.bits(pos, logval);
    uint64_t word = decode(offset, curr_popcnt);
    return sum + popcnt(word & ((1ull << (p % 63)) - 1));
}

uint64_t RRR2::rankzero(uint64_t p) const {
	return p - rank(p);
}


uint64_t RRR2::select(const uint64_t r) const {
	assert(r < onecnt); //onecnt cannot be 0
    uint64_t i = r + 1;
	unsigned int bits_per_sumR = onecnt == 1 ? 1 : ceillog2(onecnt);
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
	unsigned int logval = (curr_popcnt == 0 || curr_popcnt == 63) ? 1 : ceillog2(combination.word(63 * 32 + hold));
    uint64_t offset = S.bits(pos, logval);
    uint64_t word = decode(offset, curr_popcnt);
    return selectword(word, i - sum - 1)  + block * 63;
}


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


//------------------------------------------------------------------------

struct BlockIntIterator {
};

void RRR2HintSel::init() {
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

}

void RRR2HintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_
