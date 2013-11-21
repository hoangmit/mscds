#include "rank3p.h"
#include "bitop.h"
//#include "bitstream.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

namespace mscds {

uint64_t Rank3pBuilder::getwordz(const BitArray& v, size_t idx) {
	if (idx < v.word_count()) return v.word(idx);
	else return 0;
}

void Rank3pBuilder::build(const BitArray& b, Rank3p * o) {
    o->bits = b;
    uint64_t num_of_l1_l2 = b.length() % 2048 != 0 ? b.length() / 2048 + 1 : b.length() / 2048;
    o->l1_l2=BitArray::create(num_of_l1_l2 * 64);
    o->l1_l2.fillzero();
    uint64_t num_of_l0 = b.length() % (1ULL << 32) != 0 ? b.length() / (1ULL << 32) + 1 : b.length() / (1ULL << 32);
    o->l0=BitArray::create(num_of_l0 * 64);
    o->l0.word(0) = 0;
    uint64_t curr_l1_l2_blk=0, curr_l0_blk=0, curr_l1_l2_count=0, curr_block_count=0;
    o->onecnt=0;
    unsigned int blocknum=0, hold;
   
    //Initialising L1/L2
    for (uint64_t i=0; i<b.length(); ) {
        if (i + 64 <= b.length()) {
            hold=popcnt(getwordz(b, i / 64));
            i+=64;
        }
        else {
            hold=popcnt(b.bits(i, b.length() - i));
            i=b.length();
        }

        curr_l1_l2_count+=hold;
        curr_block_count+=hold;
        o->onecnt+=hold;

        if ((i % 512 == 0) || (i == b.length())) {
            if (blocknum==3) {
                ++curr_l1_l2_blk;

                if (i < b.length())
                    o->l1_l2.word(curr_l1_l2_blk) |= curr_l1_l2_count;

                blocknum=0;
            } 
            else {
                o->l1_l2.word(curr_l1_l2_blk) |= (curr_block_count << (32 + 10 * blocknum));
                ++blocknum;
            }

            curr_block_count=0;
        }

        if ((i < b.length()) && (i % (1ULL << 32) == 0)) {
            curr_l1_l2_count=0;
            o->l0.word(++curr_l0_blk)=o->onecnt;
        }
    }

    uint64_t num_of_sampling = b.length() % 8192 != 0 ? b.length() / 8192 + 1 : b.length() / 8192;
    o->sampling=BitArray::create(num_of_sampling * 32);
    o->sampling.fillzero();
    uint64_t curr_l0_count, curr_sampling_idx=0, l0_sampling_offset, l1_blk=0;
    uint64_t l1_rank, bitpos;
    unsigned int gap;

    for (uint64_t i=0; i < o->l0.word_count(); ++i) {
        curr_l0_count = i < (o->l0.word_count() - 1) ? o->l0rank(i+1) - o->l0rank(i) : o->onecnt - o->l0rank(i);
        l0_sampling_offset=0;

        for (uint64_t j=1; j<=curr_l0_count; j+=8192) {
            if (i*2097152+2097151 < o->l1_l2.word_count())
                l1_blk=o->l1binarysearch(j, l1_blk, i*2097152+2097151);
            else
                l1_blk=o->l1binarysearch(j, l1_blk, o->l1_l2.word_count()-1);

            bitpos = l1_blk*2048;
            l1_rank = o->l1rank(l1_blk);
            
            while (l1_rank < j) {
                if (j - l1_rank < 64)
                    gap = j - l1_rank;
                else
                    gap = 64;

                l1_rank += popcnt(b.bits(bitpos, gap));
                bitpos += gap;
            }

            o->sampling.setbits(curr_sampling_idx + l0_sampling_offset, (bitpos - 1) % (1ULL << 32), 32);
            l0_sampling_offset+=32;
        }

        curr_sampling_idx+=16777216; //Number of sampling answers for 1 L0 block is 524288 => 1 L0 block will take 16777216 bits for sampling answers
    }

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

//Does not initialize sampling answers
void Rank3pBuilder::build(const BitArray &b, OArchive &ar) {
	ar.startclass("Rank3p", 1);
	ar.var("bit_len").save(b.length());
	ar.var("inventory");
    uint64_t num_of_l1_l2 = b.length() % 2048 != 0 ? b.length() / 2048 + 1 : b.length() / 2048;
    BitArray l1_l2=BitArray::create(num_of_l1_l2 * 64);
    l1_l2.fillzero();
    uint64_t num_of_l0 = b.length() % (1ULL << 32) != 0 ? b.length() / (1ULL << 32) + 1 : b.length() / (1ULL << 32);
    BitArray l0=BitArray::create(num_of_l0 * 64);
    l0.word(0)=0;
    uint64_t curr_l1_l2_blk=0, curr_l0_blk=0, curr_l1_l2_count=0, curr_block_count=0, cnt=0;
    unsigned int blocknum=0, hold;
   
    //Initialising L1/L2
    for (uint64_t i=0; i<b.length(); ) {
        if (i + 64 <= b.length()) {
            hold=popcnt(getwordz(b, i / 64));
            i+=64;
        }
        else {
            hold=popcnt(b.bits(i, b.length() - i));
            i=b.length();
        }

        curr_l1_l2_count+=hold;
        curr_block_count+=hold;
        cnt+=hold;

        if ((i % 512 == 0) || (i == b.length())) {
            if (blocknum==3) {
                ++curr_l1_l2_blk;

                if (i < b.length())
                    l1_l2.word(curr_l1_l2_blk) |= curr_l1_l2_count;

                blocknum=0;
            } 
            else {
                l1_l2.word(curr_l1_l2_blk) |= (curr_block_count << (32 + 10 * blocknum));
                ++blocknum;
            }

            curr_block_count=0;
        }

        if ((i < b.length()) && (i % (1ULL << 32) == 0)) {
            curr_l1_l2_count=0;
            l0.word(++curr_l0_blk)=cnt;
        }
    }

    l1_l2.save(ar.var("l1_l2"));
    l0.save(ar.var("l0"));
	ar.var("onecnt").save(cnt);
	ar.endclass();

	/*ar.startclass("Rank3p", 1);
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

void Rank3p::savep(OArchive &ar) const {
	ar.startclass("Rank3p_rankonly", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
    l1_l2.save(ar.var("l1_l2"));
	l0.save(ar.var("l0"));
	ar.var("onecnt").save(onecnt);
	ar.endclass();
}

void Rank3p::loadp(IArchive &ar, BitArray &b) {
	ar.loadclass("Rank3p_rankonly");
	size_t blen;
	ar.var("bit_len").load(blen);
	if (b.length() != blen) throw std::runtime_error("length mismatch");
	//uint64_t nc = ((blen + 2047) / 2048) * 2;
    l1_l2.load(ar.var("l1_l2"));
	l0.load(ar.var("l0"));
	this->bits = b;
	ar.var("onecnt").load(onecnt);
	ar.endclass();
}

void Rank3p::save(OArchive &ar) const {
	ar.startclass("Rank3p", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
	l1_l2.save(ar.var("l1_l2"));
    l0.save(ar.var("l0"));
	ar.var("onecnt").save(onecnt);
	bits.save(ar.var("bits"));
	ar.endclass();
}

void Rank3p::load(IArchive &ar) {
	ar.loadclass("Rank3p");
	size_t blen;
	ar.var("bit_len").load(blen);
	l1_l2.load(ar.var("l1_l2"));
    l0.load(ar.var("l0"));
	ar.var("onecnt").load(onecnt);
	bits.load(ar.var("bits"));
	ar.endclass();
	if (bits.length() != blen) throw std::runtime_error("length mismatch");
}



bool Rank3p::bit(uint64_t p) const {
	return bits.bit(p);
}

uint64_t Rank3p::rank(const uint64_t p) const {
	assert(p <= bits.length());
	if (p == bits.length()) return onecnt;
    uint64_t l0_blk = p / (1ULL << 32);
    uint64_t l1_blk = p / 2048;
    uint64_t l0_rank=l0rank(l0_blk);
    uint64_t l1_rank=l1rank(l1_blk);
    uint64_t block_sum=0;
    uint64_t curr_idx;
   
    for (curr_idx=l1_blk*2048; curr_idx+512<=p; curr_idx+=512)
        block_sum+=basicblkcount(curr_idx / 512);

    return countone(curr_idx, p) + block_sum + l0_rank + l1_rank;
        
/*	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 10) & ~1ull;

	uint64_t val = (inv.word(blk) & 0x3FFFFFFFFFFFFULL) + subblkrank(blk, ((p >> 8) & 7ULL));
	for (unsigned int i = 0; i < (unsigned int) (wpos & 3ULL); ++i)
		val += popcnt(bits.word(i + (wpos & ~3ULL)));
	return val + word_rank(wpos, p & 63ULL);*/
}

uint64_t Rank3p::rankzero(uint64_t p) const {
	return p - rank(p);
}

uint64_t Rank3p::l0rank(uint64_t blk) const {
    return l0.word(blk);
}

uint64_t Rank3p::l1rank(uint64_t blk) const {
    return l1_l2.bits(blk * 64, 32);
}

//start must be word-aligned
uint64_t Rank3p::countone(uint64_t start, uint64_t end) const {
    uint64_t hold=0;

    for (uint64_t i=start; i<end; ) {
        if (i + 64 <= end) {
            hold+=popcnt(bits.word(i / 64));
            i+=64;
        }
        else {
            hold+=popcnt(bits.bits(i, end-i));
            i=end;
        }
    }

    return hold;
}

uint64_t Rank3p::basicblkcount(uint64_t blk) const {
    int blocknum = blk % 4;
    uint64_t l1_block = blk / 4;
    uint64_t l0_block = l1_block / 2097152;
    uint64_t num_of_basic_blk = bits.length() % 512 != 0 ? bits.length() / 512 + 1 : bits.length() / 512;

    if (blocknum == 3) {
        if (blk == num_of_basic_blk - 1)
            return onecnt - l0rank(l0_block) - l1rank(l1_block) - l1_l2.bits(l1_block * 64 + 32, 10) - l1_l2.bits(l1_block * 64 + 42, 10) - l1_l2.bits(l1_block * 64 + 52, 10);

        return l1rank(l1_block + 1) - l1rank(l1_block) - l1_l2.bits(l1_block * 64 + 32, 10) - l1_l2.bits(l1_block * 64 + 42, 10) - l1_l2.bits(l1_block * 64 + 52, 10);
    }

    return l1_l2.bits(l1_block * 64 + 32 + blocknum * 10, 10);
}

//Binary search for select(j) in L1 layer from block start to block end within the same L0 block
uint64_t Rank3p::l1binarysearch(uint64_t j, uint64_t start, uint64_t end) const {
    uint64_t avg=0;
    uint64_t curr_l0_blk = end / 2097152;

    while (start <= end) {
        avg = (end + start) / 2;

        //If current L1 block is the last block of the whole bit array
        if (avg == l1_l2.word_count() - 1)  {
            if (l1rank(avg) < j && (onecnt - l0rank(curr_l0_blk)) >= j)
                break;
        }
        //If current L1 block is not the last block within the given L0 block
        else if (avg < curr_l0_blk * 2097152 + 2097151) {
            if (l1rank(avg) < j && l1rank(avg+1) >= j)
                break;
        }
        else if (l1rank(avg) < j &&  (l0rank(curr_l0_blk+1) - l0rank(curr_l0_blk)) >= j)
            break;

        if (l1rank(avg) >= j)
            end=avg-1;
        else
            start=avg+1;
    }

    return avg;
}

//Binary search for select(i) in L0 layer
uint64_t Rank3p::l0binarysearch(uint64_t i) const {   
    uint64_t start=0, end=l0.word_count()-1, avg=0;

    while (start <= end) {
        avg = (end + start) / 2;

        if (avg < (l0.word_count() - 1)) {
            if (l0rank(avg) < i && l0rank(avg+1) >= i)
                break;
        }
        else if (l0rank(avg) < i && onecnt >= i)
            break;

        if (l0rank(avg) >= i)
            end=avg-1;
        else
            start=avg+1;
    }

    return avg;
}

/*uint64_t Rank3p::blkrank(size_t blk) const {
	return inv.word(2*blk) & 0x3FFFFFFFFFFFFULL;
}

uint64_t Rank3p::subblkrank(size_t blk, unsigned int off) const {
	// off = [0..7]
	const unsigned int hi = inv.word(blk) >> 50;
	off = (off - 1) & 7;
	uint64_t subblk_rank = (inv.word(blk + 1) >>  off * 9) & 0x1FFULL;
	subblk_rank |= ((hi >> off * 2) & 3ULL) << 9;
	return subblk_rank;
} */

uint64_t Rank3p::select(const uint64_t r) const {
	assert(r < onecnt);
    uint64_t i=r+1;
    
    uint64_t l0_blk=l0binarysearch(i); //Binary search L0 index
    i-=l0rank(l0_blk);
    uint64_t block_of_8192=(i-1)/8192;
    if ((i - 1) % 8192 == 0) return sampling.bits((l0_blk*524288+block_of_8192)*32, 32)+(l0_blk*2097152);
    uint64_t l1_blk=sampling.bits((l0_blk*524288+block_of_8192)*32, 32)/2048+(l0_blk*2097152); //Get L1 index from nearest sampling answer
    l1_blk+=(i-l1rank(l1_blk)-1)/2048; //Skip over L1 blocks if the difference in values is greater than 1 L1 block size
    uint64_t curr_l0_count = l0_blk < (l0.word_count() - 1) ? l0rank(l0_blk+1) - l0rank(l0_blk) : onecnt - l0rank(l0_blk);

    if ((block_of_8192 + 1) * 8192 < curr_l0_count)
        l1_blk=l1binarysearch(i, l1_blk, sampling.bits((l0_blk*524288+block_of_8192+1)*32, 32)/2048 + (l0_blk*2097152));
    else if (l0_blk*2097152+2097151 < l1_l2.word_count()) //Binary search to the correct L1 index
        l1_blk=l1binarysearch(i, l1_blk, l0_blk*2097152+2097151); 
    else
        l1_blk=l1binarysearch(i, l1_blk, l1_l2.word_count()-1);

    i-=l1rank(l1_blk);
    uint64_t basic_blk=l1_blk*4;
    uint64_t num_of_basic_blk = bits.length() % 512 != 0 ? bits.length() / 512 + 1 : bits.length() / 512;
    uint64_t curr_blk_count;

    while (basic_blk<num_of_basic_blk) { //Go to the correct basic block
        curr_blk_count=basicblkcount(basic_blk);

        if (i <= curr_blk_count)
            break;

        i-=curr_blk_count;
        ++basic_blk;
    }

    uint64_t bitidx=basic_blk * 512, curr_word;
    unsigned int step, wordcnt;

    while (i > 0) { //Find the exact bit
        if (bitidx + 64 <= bits.length()) {
            curr_word=bits.word(bitidx / 64);
            step=64;
        }
        else {
            step=bits.length()-bitidx;
            curr_word=bits.bits(bitidx, step);
        }

        wordcnt=popcnt(curr_word);

        if (i <= wordcnt) {
            bitidx+=selectword(curr_word, i-1) + 1;
            break;
        }
        
        i-=wordcnt;
        bitidx+=step;
    }

    return bitidx - 1;

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

/*uint64_t Rank3p::selectblock(uint64_t blk, uint64_t d) const {
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

uint64_t Rank3p::blkrank0(size_t blk) const {
	return blk * 2048 - (inv.word(blk*2) & 0x3FFFFFFFFFFFFULL);
}

uint64_t Rank3p::subblkrank0(size_t blk, unsigned int off) const {
	return off*4*64 - subblkrank(blk, off);
}*/

uint64_t Rank3p::selectzero(const uint64_t r) const {
	assert(r < bits.length() - onecnt);
    uint64_t start = 1, end = bits.length(), avg;

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

/*uint64_t Rank3p::selectblock0(uint64_t lo, uint64_t d) const {
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

void Rank3p::clear() {
    l0.clear();
	l1_l2.clear();
	bits.clear();
	onecnt = 0;
}

/*unsigned int Rank3p::word_rank(size_t idx, unsigned int i) const {
	return (i > 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}*/

//------------------------------------------------------------------------

struct BlockIntIterator {
/*	const Rank3p& r;
	uint64_t p;
	BlockIntIterator(const Rank3p& _r): r(_r), p(0) {}
	void operator++() { ++p; }
	uint64_t operator*() const { return r.blkrank(p); }*/
};

void Rank3pHintSel::init() {
/*	hints.clear();
	BlockIntIterator it(rankst);
	// every 4096 positions
	hints = bsearch_hints(it, rankst.inv.word_count()/2, rankst.one_count(), 12); */
}

void Rank3pHintSel::init(Rank3p& r) {
	hints.clear();
	this->rankst = r;
	init();
}

void Rank3pHintSel::init(BitArray& b) {
	hints.clear();
	//Rank3pBuilder bd;
	Rank3pBuilder::build(b, &rankst);
	init();
}

uint64_t Rank3pHintSel::select(uint64_t r) const {
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

void Rank3pHintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_
