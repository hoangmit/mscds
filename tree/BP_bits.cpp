#include "BP_bits.h"

#include <stack>
#include <utility>
#include <vector>

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
		uint64_t ed1 = std::min(pos + 8, endp);
		for (; pos < ed1; pos++) {
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



}//namespace