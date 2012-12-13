#include "BP_bits.h"

#include <stack>
#include <utility>
#include <vector>
#include <sstream>



namespace mscds {

std::vector<size_t> find_pioneers_v(const BitArray& bp, size_t blksize) {
	std::stack<size_t> opens;
	std::pair<size_t, size_t> lastfar;
	bool haslast = false;
	std::vector<size_t> ret;

	size_t pioneer_cnt = 0;
	for (size_t i = 0; i < bp.length(); ++i) {
		if (bp[i]) {
			opens.push(i);
		} else {
			size_t op = opens.top();
			opens.pop();
			if (op/blksize != i/blksize) { // far
				if (haslast && lastfar.first/blksize != op/blksize) {
					// pioneer
					ret.push_back(lastfar.first);
					ret.push_back(lastfar.second);
					pioneer_cnt++;
				}
				haslast = true;
				lastfar = std::make_pair(op, i);
			}
		}
		if ((i + 1)/blksize != i/blksize) {
			if (haslast) {
				// pioneer
				ret.push_back(lastfar.first);
				ret.push_back(lastfar.second);
				pioneer_cnt++;
			}
			haslast = false;
		}
	}
	assert(opens.empty());
	if (haslast) {
		// pioneer
		ret.push_back(lastfar.first);
		ret.push_back(lastfar.second);
		pioneer_cnt++;
	}
	assert(pioneer_cnt <= 4 * ((bp.length() + blksize - 1)/blksize) - 6);
	std::sort(ret.begin(), ret.end());
	return ret;

}

BitArray find_pioneers(const BitArray& bp, size_t blksize) {
	std::vector<size_t> v = find_pioneers_v(bp, blksize);
	BitArray ret = BitArray::create(bp.length());
	ret.fillzero();
	for (int i = 0; i < v.size(); i++)
		ret.setbit(v[i], true);
	return ret;
}

//---------------------------------------------------------------------------------------

int8_t BP_block::min_excess8_c(uint8_t c)  {
	int8_t e = 0, min = 120;
	for (int i = 0; i < 8; i++) {
		if (c & 1) e++;
		else e--;
		if (min > e) min = e;
		c >>= 1;
	}
	return min;
}

int8_t BP_block::min_revex8_c(uint8_t c)  {
	int8_t e = 0, max = -120;
	for (int i = 0; i < 8; i++) {
		if (c & 128) e++;
		else e--;
		if (max < e) max = e;
		c <<= 1;
	}
	return max;
}

int8_t BP_block::excess8_c(uint8_t c) {
	int8_t e = 0;
	for (int i = 0; i < 8; i++) {
		if (c & 1) e++;
		else e--;
		c >>= 1;
	}
	return e;
}

int8_t BP_block::min_op_ex_pos8_c(uint8_t c) {
	int8_t e = 0, min = 100, mpos = -1;
	for (int i = 0; i < 8; i++) {
		if (c & 1) {
			e++;
			if (min >= e) { min = e; mpos = i; }
		} else e--;
		c >>= 1;
	}
	return mpos;
}


int8_t BP_block::min_excess8_t[256] =  {
	-8,-6,-6,-4,-6,-4,-4,-2,-6,-4,-4,-2,-4,-2,-2, 0,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0,-1, 1,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-7,-5,-5,-3,-5,-3,-3,-1,-5,-3,-3,-1,-3,-1,-1, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1,
	-4,-2,-2, 0,-2, 0,-1, 1,-3,-1,-1, 1,-2, 0,-1, 1};

int8_t BP_block::excess8_t[256] = {
	-8,-6,-6,-4,-6,-4,-4,-2,-6,-4,-4,-2,-4,-2,-2, 0,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0, 0, 2,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0, 0, 2,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0, 0, 2,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-2, 0, 0, 2, 0, 2, 2, 4, 0, 2, 2, 4, 2, 4, 4, 6,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0, 0, 2,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-2, 0, 0, 2, 0, 2, 2, 4, 0, 2, 2, 4, 2, 4, 4, 6,
	-4,-2,-2, 0,-2, 0, 0, 2,-2, 0, 0, 2, 0, 2, 2, 4,
	-2, 0, 0, 2, 0, 2, 2, 4, 0, 2, 2, 4, 2, 4, 4, 6,
	-2, 0, 0, 2, 0, 2, 2, 4, 0, 2, 2, 4, 2, 4, 4, 6,
	 0, 2, 2, 4, 2, 4, 4, 6, 2, 4, 4, 6, 4, 6, 6, 8};

int8_t BP_block::min_op_ex_pos8_t[256] = {
	-1, 0, 1, 0, 2, 2, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 4, 4, 4, 4, 4, 4, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 0, 2, 2, 1, 0,
	 4, 4, 4, 4, 4, 4, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 0,
	 6, 6, 6, 6, 6, 6, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 0, 2, 2, 1, 0,
	 4, 4, 4, 4, 4, 4, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0,
	 7, 7, 7, 7, 7, 7, 7, 0, 7, 7, 7, 0, 2, 2, 1, 0,
	 7, 7, 7, 7, 7, 7, 7, 0, 7, 7, 7, 0, 2, 2, 1, 0,
	 4, 4, 4, 4, 4, 4, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 0,
	 6, 6, 6, 6, 6, 6, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0,
	 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 0, 2, 2, 1, 0,
	 4, 4, 4, 4, 4, 4, 1, 0, 3, 3, 3, 0, 2, 2, 1, 0};

int8_t BP_block::min_op_ex8_t[256] = {
	99, 1, 0, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-4,-2,-2, 0,-2, 0, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-4,-2,-2, 0,-2, 0, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-6,-4,-4,-2,-4,-2,-2, 0,-4,-2,-2, 0,-2, 0, 0, 1,
	-4,-2,-2, 0,-2, 0, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-4,-2,-2, 0,-2, 0, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-5,-3,-3,-1,-3,-1,-1, 1,-3,-1,-1, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-4,-2,-2, 0,-2, 0, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1,
	-3,-1,-1, 1,-1, 1, 0, 1,-2, 0, 0, 1,-1, 1, 0, 1};

uint64_t BP_block::forward_scan(uint64_t pos, int64_t excess) const {
	assert(excess <= 0);
	assert(excess != 0 || bp.bit(pos));
	uint64_t endp = std::min(bp.length(), (pos / blksize + 1) * blksize);
	if ((pos & 7) != 0) {
		uint8_t cbyte = bp.byte(pos >> 3) >> (pos & 7);
		uint64_t ed1 = std::min(endp, ((pos >> 3) + 1) << 3);
		for (; pos < ed1; pos++) {
			if (cbyte & 1) excess--;
			else excess++;
			if (excess == 0) return pos;
			cbyte >>= 1;
		}
	}
	for (; pos + 8 <= endp; pos += 8) {
		uint8_t cbyte = bp.byte(pos >> 3);
		if (excess - min_excess8_t[cbyte] < 0)
			excess -= excess8_t[cbyte];
		else break;
	} 
	if (pos < endp) {
		uint8_t cbyte = bp.byte(pos >> 3);
		for (; pos < endp; pos++) {
			if (cbyte & 1) excess--;
			else excess++;
			if (excess == 0) return pos;
			cbyte >>= 1;
		}
	}
	return NOTFOUND;
}

uint64_t BP_block::backward_scan(uint64_t pos, int64_t excess) const {
	assert(excess >= 0);
	assert(excess != 0 || (!bp.bit(pos)));
	uint64_t endp = (pos / blksize) * blksize;
	if (((pos + 1) & 7) != 0) {
		uint8_t cbyte = bp.byte(pos >> 3) << (7 - (pos & 7));
		for (size_t i = 0; i <= (pos & 7); i++) {
			if (cbyte & 128) excess--;
			else excess++;
			if (excess == 0) return pos - i;
			cbyte <<= 1;
		}
		pos = pos - (pos & 7);
	}else pos += 1;
	assert((pos & 7) == 0);
	for (; pos > endp; pos -= 8) {
		uint8_t cbyte = ~revbits(bp.byte((pos >> 3) - 1));
		if (excess + min_excess8_t[cbyte] > 0)
			excess += excess8_t[cbyte];
		else break;
	}
	if (pos > endp) {
		uint8_t cbyte = bp.byte((pos >> 3) - 1);
		for (; pos > endp; pos--) {
			if (cbyte & 128) excess--;
			else excess++;
			if (excess == 0) return pos-1;
			cbyte <<= 1;
		}
	}
	return NOTFOUND;
}

uint64_t BP_block::min_excess_pos(uint64_t pos, uint64_t endp) const {
	assert(endp <= bp.length());
	if (pos >= endp) return BP_block::NOTFOUND;
	assert(pos/blksize == (endp - 1)/blksize);
	bool flag = false;
	uint8_t sc = 0;
	int64_t excess = 0, minex = endp - pos + 2, minpos = BP_block::NOTFOUND;
	if ((pos & 7) != 0) {
		uint8_t cbyte = bp.byte(pos >> 3) >> (pos & 7);
		uint64_t ed1 = std::min(endp, ((pos >> 3) + 1) << 3);
		for (; pos < ed1; pos++) {
			if (cbyte & 1) {
				excess++;
				if (excess <= minex) { minex = excess; minpos = pos; }
			} else excess--;
			cbyte >>= 1;
		}
	}
	for (; pos + 8 <= endp; pos += 8) {
		uint8_t cbyte = bp.byte(pos >> 3);
		if (cbyte != 0) {
			int8_t mv = min_op_ex8_t[cbyte];
			if (excess + mv <= minex) {
				minex = excess + mv; minpos = pos; flag = true; sc = cbyte;
			}
		}
		excess += excess8_t[cbyte];
	}
	if (pos < endp) {
		uint8_t cbyte = bp.byte(pos >> 3);
		for (; pos < endp; pos++) {
			if (cbyte & 1){
				excess++;
				if (excess <= minex) { minex = excess; minpos = pos; flag = false; }
			} else excess--;
			cbyte >>= 1;
		}
	}
	if (minex > excess || minpos == BP_block::NOTFOUND)
		return BP_block::NOTFOUND;
	if (!flag) return minpos;
	else return minpos + min_op_ex_pos8_t[sc];
}


uint64_t BP_block::forward_scan_slow(uint64_t pos, int64_t excess) const {
	assert(excess <= 0);
	if (excess == 0 && (!bp.bit(pos))) return NOTFOUND;
	uint64_t endp = std::min(bp.length(), (pos / blksize + 1) * blksize);
	int64_t e = 0;
	for (size_t i = pos; i < endp; i++) {
		if (bp.bit(i)) e += 1;
		else e -= 1;
		if (e == excess) return i;
	}
	return NOTFOUND;
}

uint64_t BP_block::backward_scan_slow(uint64_t pos, int64_t excess) const {
	assert(excess >= 0);
	if (excess == 0 && (bp.bit(pos))) return NOTFOUND;
	int64_t endp = (pos / blksize) * blksize;
	int64_t e = 0;
	for (int64_t i = pos; i >= endp; i--) {
		if (bp.bit(i)) e += 1;
		else e -= 1;
		if (e == excess) return i;
	}
	return NOTFOUND;
}

uint64_t BP_block::min_excess_pos_slow(uint64_t l, uint64_t r) const {
	int64_t excess = 0, minex = r - l + 1, minpos = BP_block::NOTFOUND;
	for (uint64_t i = l; i < r; i++) {
		if (bp[i]) {
			excess++;
			if (excess <= minex) { minex = excess; minpos = i; }
		} else excess--;
	}
	if (minex <= excess) return minpos;
	else return BP_block::NOTFOUND;
}


//---------------------------------------------------------------------------------------

void BP_aux::build(const BitArray& bp, unsigned int blksize) {
	assert(blksize >= 4);
	this->blksize = blksize;
	bp_bits = bp;
	blk.init(bp, blksize);
	Rank6pBuilder::build(bp, &bprank);
	if (bp.length() > blksize) {
		std::vector<uint64_t> pio = find_pioneers_v(bp, blksize);
		//pioneer_map
		pioneer_map.build(pio);
		//nextlvl
		BitArray nxtlvl = BitArray::create(pio.size());
		for (size_t i = 0; i < pio.size(); i++)
			nxtlvl.setbit(i, bp[pio[i]]);
		lowerlvl = new BP_aux();
		lowerlvl->build(nxtlvl, blksize);
	} else {
		lowerlvl = NULL;
	}
	//std::cout << this->to_str() << std::endl;
}

void BP_aux::clear() {
	blk.clear();
	bprank.clear();
	bp_bits.clear();
	pioneer_map.clear();
	delete lowerlvl;
	blksize = 0;
}

uint64_t BP_aux::find_close(uint64_t p) const {
	assert(p < length());
	if (!bit(p)) return p;
	uint64_t pc = blk.forward_scan(p, 0);
	if (pc != BP_block::NOTFOUND) return pc;
	assert(length() > blksize);
	// pioneer in the left i.e. pioneers.select(pio_rank) <= p
	uint64_t pio_rank = pioneer_map.rank(p+1) - 1;
	uint64_t pio = pioneer_map.select(pio_rank);
	uint64_t pio_close = pioneer_map.select(lowerlvl->find_close(pio_rank));
	assert(!bp_bits[pio_close]);
	if (pio == p) return pio_close; //(not necessary but this skips the scanning)
	int64_t diff = excess(p) - excess((pio_close/blksize)*blksize);
	uint64_t ret = blk.forward_scan((pio_close/blksize) * blksize, diff);
	assert(!bp_bits[ret]);
	return ret;
}

uint64_t BP_aux::find_open(uint64_t p) const {
	assert(p < length());
	if (bit(p)) return p;
	uint64_t po = blk.backward_scan(p, 0);
	if (po != BP_block::NOTFOUND) return po;
	assert(length() > blksize);
	// pioneer in the right i.e. pioneers.select(pio_rank >= p);
	uint64_t pio_rank = pioneer_map.rank(p);
	uint64_t pio = pioneer_map.select(pio_rank);
	uint64_t open_pio = pioneer_map.select(lowerlvl->find_open(pio_rank));
	assert(bp_bits[open_pio]);
	if (pio == p) return open_pio;
	int64_t diff = excess((open_pio/blksize + 1)*blksize) - excess(p + 1);
	uint64_t ret = blk.backward_scan((open_pio/blksize + 1) * blksize - 1, diff);
	assert(bit(ret));
	return ret;
}

uint64_t BP_aux::enclose(uint64_t p) const {
	if (p >= length()) return BP_block::NOTFOUND;
	if (!bit(p)) return find_open(p);
	if (p == 0) return BP_block::NOTFOUND;
	uint64_t pe = BP_block::NOTFOUND;
	if (p % blksize != 0) pe = blk.backward_scan(p - 1, 1);
	if (pe != BP_block::NOTFOUND || p / blksize == 0) return pe;
	// pioneer in the right if it exists
	uint64_t pio_rank = pioneer_map.rank(p);
	uint64_t open_pio_rank = lowerlvl->enclose(pio_rank);
	if (open_pio_rank == BP_block::NOTFOUND) return BP_block::NOTFOUND;
	uint64_t open_pio = pioneer_map.select(open_pio_rank);
	assert(bp_bits[open_pio]);
	if (excess(open_pio) + 1 == excess(p)) return open_pio;
	int64_t diff = excess((open_pio/blksize + 1)*blksize) - excess(p) + 1;
	uint64_t ret = blk.backward_scan((open_pio/blksize + 1) * blksize - 1, diff);
	assert(bit(ret));
	return ret;
}

std::string BP_aux::to_str() const {
	assert(length() < (1UL << 16));
	std::ostringstream ss;
	for (size_t i = 0; i < length(); i++) {
		if (i % blksize == 0 && i > 0) ss << '-';
		if (bit(i)) ss << '(';
		else ss << ')';
	}
	ss << '\n';
	if (pioneer_map.one_count() > 0) {
		size_t last = lowerlvl->length();
		size_t p = 0;
		size_t next = pioneer_map.select(p); 
		for (size_t i = 0; i < length(); i++) {
			if (i % blksize == 0 && i > 0) ss << ' ';
			if (i < next) ss << ' ';
			else {
				ss << '1';
				p++;
				if (p == last) next = length() + 1;
				else next = pioneer_map.select(p);
			}
		}
	}
	return ss.str();
}

}//namespace
