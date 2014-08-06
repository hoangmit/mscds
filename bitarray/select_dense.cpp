
#include "select_dense.h"


namespace mscds {

unsigned int Block::_numbit(unsigned int p) {
	if (p == 0) return 0;
	else return ceillog2(p + 1);
}

bool Block::_check_span(unsigned int span, const std::vector<unsigned int> &inp, const unsigned int max_diff) {
	assert(span > 0);
	if (span > 1)
		for (unsigned int i = 0; i < BLK_COUNT; i += span) {
			auto last = std::min(i + span - 1, BLK_COUNT - 1);
			if (inp[last] - inp[i] > max_diff) return false;
		}
	return true;
}

std::vector<unsigned int> Block::_make_span(unsigned int span, const std::vector<unsigned int> &inp, unsigned int &width) {
	std::vector<unsigned int> ret((BLK_COUNT + span - 1)/span);
	width = 0;
	unsigned p = 0;
	for (unsigned int i = 0; i < BLK_COUNT; i += span) {
		ret[p++] = inp[i];
		width = std::max(width, _numbit(inp[i]));
	}
	return ret;
}

void Block::build(const std::vector<unsigned int> &_inp, OBitStream &overflow, unsigned int start_flow) {
	assert(_inp.size() <= BLK_COUNT);
	assert(std::is_sorted(_inp.begin(), _inp.end()));
	std::vector<unsigned int> inp = _inp;
	if (inp.size() == 0) {
		assert(false);
		return ;
	}
	// fill up, empty space
	while (inp.size() < BLK_COUNT) inp.push_back(inp.back() + 1);
	for (unsigned int i = 1; i < inp.size(); ++i)
		inp[i] -= inp[0];
	inp[0] = 0;

	const static unsigned int MAXD = 512;

	unsigned int z, span;
	if (_check_span(74, inp, MAXD)) {
		span = 74;
		z = 0;
	} else {
		span = 64;
		z = 1;
		while (span >= 1) {
			if (_check_span(span, inp, MAXD)) break;
			span /= 2;
			z += 1;
		}
		//
		assert(z < 8);
	}
	// got: z
	h.v1 = inp[0];
	assert(h.v1 <= MASK_1);
	unsigned int w;
	std::vector<unsigned int> vals = _make_span(span, inp, w);
	_write_case(z, w, vals, overflow, start_flow);
}

unsigned int Block::get_span() const {
	unsigned int casex = (h.v1 >> (8*7)) & 7;
	if (casex == 0) return 74;
	else return 512 >> (casex + 2);
}

unsigned int Block::get_sublen() const {
	unsigned int casex = (h.v1 >> (8*7)) & 7;
	if (casex == 0) return 7;
	else return (1u << (casex + 2));
}

void Block::_write_case(unsigned int z, unsigned int w, const std::vector<unsigned int> &vals,
							   mscds::OBitStream &overflow, unsigned int start_flow) {
	unsigned int exp = z == 0 ? 7 : (1u<< (z+2));
	assert(exp == vals.size());
	assert(0 == vals[0]);

	h.v1 &= (1ull << (8*7)) - 1;
	h.v1 |= (uint64_t)z << (8*7);

	if (z == 0) {
		assert(w <= 11);
		assert(vals.size() == 7);	
		h.v2 = 0;
		for (int i = 5; i >= 1; i--) {
			h.v2 <<= 11;
			h.v2 |= vals[i];
		}
		uint64_t v = (uint64_t)vals[6];
		h.v2 |= (v << (11*5));
		h.v1 |= (v >> 9) << 62;
	} else {
		h.v2 = start_flow;
		assert(h.v2 < (1ull << (8*7)));
		h.v2 |= (uint64_t)w << (8*7);

		for (unsigned int i = 0; i < vals.size(); ++i) {
			overflow.puts(vals[i], w);
		}
	}
}

uint64_t Block::blk_ptr() const {
	return h.v1 & MASK_1;
}

unsigned int Block::_get_case0(unsigned int pos) const {
	assert(pos < 7);
	if (pos == 0) return 0;
	pos -= 1;
	if (pos < 5) {
		return (h.v2 >> (pos * 11)) & ((1u << 11) - 1);
	} else {
		uint64_t v1 = (h.v2 >> (11*5));
		uint64_t v2 = ((h.v1 >> 62) << 9);
		return (unsigned int)( v1 | v2 );
	}
}

unsigned int Block::get_casex(unsigned int pos, const BitArray &v, unsigned int start) const {
	unsigned int casex = (h.v1 >> (8*7)) & 7;
	if (casex == 0) return _get_case0(pos);
	uint64_t ptr = start + (h.v2  & 0x00FFFFFFFFFFFFFFull);
	unsigned int w = h.v2 >> (8*7);
	return v.bits(ptr + pos*w, w);
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

void SelectDenseBuilder::build(const BitArray &b, SelectDense *o) {
	OBitStream header;
	OBitStream overflow;
	std::vector<unsigned int> v(Block::BLK_COUNT);
	unsigned p = 0;
	Block bx;
	for (unsigned int i = 0; i < b.length(); ++i) {
		if (b[i]) v[p++] = i;
		if (p == Block::BLK_COUNT) {
			bx.build(v, overflow, 0);
			header.puts(bx.h.v1);
			header.puts(bx.h.v2);
			p = 0;
		}
	}
	if (p > 0) {
		v.resize(p);
		bx.build(v, overflow, 0);
		header.puts(bx.h.v1);
		header.puts(bx.h.v2);
	}
	header.close();
	overflow.close();
	o->bits = b;
	header.build(&(o->ptrs));
	overflow.build(&(o->overflow));
}

uint64_t SelectDense::select(uint64_t r) const {
	uint64_t blk = r / Block::BLK_COUNT;
	uint64_t p = r % Block::BLK_COUNT;
	Block bx;
	bx.h.v1 = ptrs.word(blk*2);
	bx.h.v2 = ptrs.word(blk*2 + 1);
	uint64_t sloc = bx.blk_ptr();
	if (p == 0) return sloc;
	auto span = bx.get_span();
	auto subblk = p / span;
	auto subp = p % span;
	auto stx = bx.get_casex(subblk, overflow, 0);
	if (subp == 0) return stx;
	sloc += stx;
	//auto px = scan(bits, sloc + stx, subp);
	return 0;
}

}//namespace
