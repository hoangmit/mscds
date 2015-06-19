#include "rank3p.h"
#include "bitop.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

namespace mscds {

uint64_t Rank3pBuilder::getwordz(const BitArrayInterface* v, size_t idx) {
	if (idx < v->word_count()) return v->word(idx);
	else return 0;
}

void Rank3pBuilder::build_aux(const BitArrayInterface* b, Rank3pAux * o) {
    o->bits = b;
	uint64_t num_of_l1_l2 = b->length() % 2048 != 0 ? b->length() / 2048 + 1 : b->length() / 2048;
	o->l1_l2 = BitArrayBuilder::create(num_of_l1_l2 * 64);
    o->l1_l2.fillzero();
	uint64_t num_of_l0 = b->length() % (1ULL << 32) != 0 ? b->length() / (1ULL << 32) + 1 : b->length() / (1ULL << 32);
	o->l0 = BitArrayBuilder::create(num_of_l0 * 64);
    o->l0.setword(0, 0);
    uint64_t curr_l1_l2_blk=0, curr_l0_blk=0, curr_l1_l2_count=0, curr_block_count=0;
    o->onecnt=0;
    unsigned int blocknum=0, hold;
   
    //Initialising L1/L2
	for (uint64_t i = 0; i<b->length();) {
		if (i + 64 <= b->length()) {
            hold=popcnt(getwordz(b, i / 64));
            i+=64;
        }
        else {
			hold = popcnt(b->bits(i, b->length() - i));
			i = b->length();
        }

        curr_l1_l2_count+=hold;
        curr_block_count+=hold;
        o->onecnt+=hold;

		if ((i % 512 == 0) || (i == b->length())) {
            if (blocknum==3) {
                ++curr_l1_l2_blk;

				if (i < b->length()) {
					uint64_t v = o->l1_l2.word(curr_l1_l2_blk);
					v |= curr_l1_l2_count;
					o->l1_l2.setword(curr_l1_l2_blk, v);
				}

                blocknum=0;
            } 
            else {
				uint64_t v = o->l1_l2.word(curr_l1_l2_blk);
                v |= (curr_block_count << (32 + 10 * blocknum));
				o->l1_l2.setword(curr_l1_l2_blk, v);
                ++blocknum;
            }

            curr_block_count=0;
        }

		if ((i < b->length()) && (i % (1ULL << 32) == 0)) {
            curr_l1_l2_count=0;
            o->l0.setword(++curr_l0_blk,o->onecnt);
        }
    }

	uint64_t num_of_sampling = b->length() % 8192 != 0 ? b->length() / 8192 + 1 : b->length() / 8192;
	o->sampling = BitArrayBuilder::create(num_of_sampling * 32);
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

				l1_rank += popcnt(b->bits(bitpos, gap));
                bitpos += gap;
            }

            o->sampling.setbits(curr_sampling_idx + l0_sampling_offset, (bitpos - 1) % (1ULL << 32), 32);
            l0_sampling_offset+=32;
        }

        curr_sampling_idx+=16777216; //Number of sampling answers for 1 L0 block is 524288 => 1 L0 block will take 16777216 bits for sampling answers
    }
}

void Rank3pBuilder::build(const BitArray &b, Rank3p *o) {
	o->_own_bits = b;
	build_aux(&b, o);
}

void Rank3pAux::save_aux(OutArchive &ar) const {
	ar.startclass("Rank3p_rankonly", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
    l1_l2.save(ar.var("l1_l2"));
	l0.save(ar.var("l0"));
	ar.var("onecnt").save(onecnt);
	ar.endclass();
}

void Rank3pAux::load_aux(InpArchive &ar, const BitArrayInterface *b) {
	ar.loadclass("Rank3p_rankonly");
	size_t blen;
	ar.var("bit_len").load(blen);
	if (b->length() != blen) throw std::runtime_error("length mismatch");
	//uint64_t nc = ((blen + 2047) / 2048) * 2;
    l1_l2.load(ar.var("l1_l2"));
	l0.load(ar.var("l0"));
	ar.var("onecnt").load(onecnt);
	ar.endclass();
	this->bits = b;
}

void Rank3p::save(OutArchive &ar) const {
	ar.startclass("Rank3p");
	_own_bits.save(ar.var("bits"));
	save_aux(ar);
	ar.endclass();
}

void Rank3p::load(InpArchive &ar) {
	ar.loadclass("Rank3p");
	_own_bits.load(ar.var("bits"));
	load_aux(ar, &_own_bits);
	ar.endclass();
}



bool Rank3pAux::bit(uint64_t p) const {
	return bits->bit(p);
}

uint64_t Rank3pAux::rank(const uint64_t p) const {
	assert(p <= bits->length());
	if (p == bits->length()) return onecnt;
    uint64_t l0_blk = p / (1ULL << 32);
    uint64_t l1_blk = p / 2048;
    uint64_t l0_rank=l0rank(l0_blk);
    uint64_t l1_rank=l1rank(l1_blk);
    uint64_t block_sum=0;
    uint64_t curr_idx;
   
    for (curr_idx=l1_blk*2048; curr_idx+512<=p; curr_idx+=512)
        block_sum+=basicblkcount(curr_idx / 512);

    return countone(curr_idx, p) + block_sum + l0_rank + l1_rank;        
}

uint64_t Rank3pAux::rankzero(uint64_t p) const {
	return p - rank(p);
}

uint64_t Rank3pAux::l0rank(uint64_t blk) const {
    return l0.word(blk);
}

uint64_t Rank3pAux::l1rank(uint64_t blk) const {
    return l1_l2.bits(blk * 64, 32);
}

//start must be word-aligned
uint64_t Rank3pAux::countone(uint64_t start, uint64_t end) const {
    uint64_t hold=0;

    for (uint64_t i=start; i<end; ) {
        if (i + 64 <= end) {
			hold += bits->popcntw(i / 64);
            i+=64;
        }
        else {
			hold += popcnt(bits->bits(i, end-i));
            i=end;
        }
    }

    return hold;
}

uint64_t Rank3pAux::basicblkcount(uint64_t blk) const {
    int blocknum = blk % 4;
    uint64_t l1_block = blk / 4;
    uint64_t l0_block = l1_block / 2097152;
	uint64_t num_of_basic_blk = bits->length() % 512 != 0 ? bits->length() / 512 + 1 : bits->length() / 512;

    if (blocknum == 3) {
        if (blk == num_of_basic_blk - 1)
            return onecnt - l0rank(l0_block) - l1rank(l1_block) - l1_l2.bits(l1_block * 64 + 32, 10) - l1_l2.bits(l1_block * 64 + 42, 10) - l1_l2.bits(l1_block * 64 + 52, 10);

        return l1rank(l1_block + 1) - l1rank(l1_block) - l1_l2.bits(l1_block * 64 + 32, 10) - l1_l2.bits(l1_block * 64 + 42, 10) - l1_l2.bits(l1_block * 64 + 52, 10);
    }

    return l1_l2.bits(l1_block * 64 + 32 + blocknum * 10, 10);
}

//Binary search for select(j) in L1 layer from block start to block end within the same L0 block
uint64_t Rank3pAux::l1binarysearch(uint64_t j, uint64_t start, uint64_t end) const {
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
uint64_t Rank3pAux::l0binarysearch(uint64_t i) const {
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

uint64_t Rank3pAux::select(const uint64_t r) const {
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
	uint64_t num_of_basic_blk = bits->length() % 512 != 0 ? bits->length() / 512 + 1 : bits->length() / 512;
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
		if (bitidx + 64 <= bits->length()) {
			curr_word = bits->word(bitidx / 64);
            step=64;
        }
        else {
			step = bits->length()-bitidx;
			curr_word = bits->bits(bitidx, step);
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

}

uint64_t Rank3pAux::selectzero(uint64_t r) const {
	assert(r < bits->length() - onecnt);
	r += 1;
	uint64_t start = 0, end = bits->length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rankzero(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}


void Rank3pAux::clear() {
    l0.clear();
	l1_l2.clear();
	bits = nullptr;
	onecnt = 0;
}


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
