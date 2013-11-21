#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include <algorithm>
#include <vector>
#include <string>
#include <stdint.h>
#include <limits>


namespace mscds {

class RMQ_table_access {
public:
	//return the number of bits :))
	static void build(const std::vector<int>& values, bool _min_struct, RMQ_table_access* tbl);
	static size_t build_stream(const std::vector<int>& values, bool _min_struct, OBitStream* out);
	// for validation and testing purpose
	template<typename T>
	static size_t _slow_m_idx(size_t st, size_t ed, const std::vector<T>& values, bool _min_struct);

	void init(size_t seqlen, BitArray b);
	void init_shared(unsigned int nblk, size_t seqlen, size_t base_pos, BitArray b);
	void switch_blk(unsigned int blk);

	/**
	return the two possible min/max indexes
	*/
	std::pair<size_t, size_t> m_idx(size_t st, size_t ed) const;
	size_t size() const { return bit_size; }
private:
	static size_t build_start(size_t len, std::vector<unsigned int> * starts);
	size_t get(unsigned layer, size_t idx) const;
private:
	std::vector<unsigned int> starts;
	size_t start_pos, base_pos;
	size_t len, bit_size;
	unsigned int nblk;
	BitArray bits;
};

class RMQ_index_blk {
public:
	static void build(const std::vector<int>& wordvals, unsigned int blksize, bool _min_struct, RMQ_index_blk* out);

	/** return the list of possible min/max indexes (at most 6 elements) */
	std::vector<unsigned int> m_idx(size_t st, size_t ed) const;
private:
	void _find(unsigned int blk, unsigned int st, unsigned int ed, std::vector<unsigned int>& out) const;
	unsigned int blksize, len;
	RMQ_table_access blockrmq;
	mutable RMQ_table_access subblkrmq;
};


//---------------------------------------------------------------

inline std::vector<unsigned int> RMQ_index_blk::m_idx(size_t st, size_t ed) const {
	std::vector<unsigned int> ret;
	if (st >= ed) return ret;
	ed -= 1;
	unsigned int stb = st / blksize, stp = st % blksize;
	unsigned int edb = ed / blksize, edp = ed % blksize;
	if (stb == edb) {
		_find(stb, stp, edp + 1, ret);
	} else {
		if (stp > 0) {
			_find(stb, stp, blksize, ret);
			stb += 1;
		}
		if (edp == blksize - 1) edb += 1;
		if (stb < edb) {
			auto r = blockrmq.m_idx(stb, edb);
			_find(r.first, 0, blksize, ret);
			if (r.first != r.second)
				_find(r.second, 0, blksize, ret);
		}
		if (edp != blksize - 1)
			_find(edb, 0, edp + 1, ret); // check this index
	}
	return ret;
}

inline void RMQ_index_blk::_find(unsigned int blk, unsigned int st, unsigned int ed, std::vector<unsigned int> &out) const {
	assert(blk < len);
	subblkrmq.switch_blk(blk);
	auto p = subblkrmq.m_idx(st, ed);
	out.push_back(blksize * blk + p.first);
	if (p.first != p.second)
		out.push_back(blksize * blk + p.second);
}


template<typename T>
inline size_t RMQ_table_access::_slow_m_idx(size_t st, size_t ed, const std::vector<T>& values, bool _min_struct) {
	T mx;
	size_t idx = ed;
	if (_min_struct)
		mx = std::numeric_limits<T>::max();
	else
		mx = std::numeric_limits<T>::min();
	for (size_t i = st; i < ed; ++i) {
		if (_min_struct) {
			if (mx > values[i]) { mx = values[i]; idx = i; }
		}
		else
			if (mx < values[i]) { mx = values[i]; idx = i; }
	}
	return idx;
}

inline std::pair<size_t, size_t> RMQ_table_access::m_idx(size_t st, size_t ed) const {
	assert(ed <= len);
	if (ed - st == 1) return std::pair<size_t, size_t>(st, st);
	unsigned int d = msb_intr(ed - st);
	size_t nx = ed - (1ull << d);
	if (st != nx)
		return std::pair<size_t, size_t>(st + get(d, st), nx + get(d, nx));
	else {
		size_t val = st + get(d, st);
		return std::pair<size_t, size_t>(val, val);
	}
}

inline size_t RMQ_table_access::get(unsigned layer, size_t idx) const {
	assert(layer > 0 && idx < len);
	return (size_t)bits.bits(start_pos + starts[layer - 1] + idx * layer, layer);
}

inline void RMQ_table_access::switch_blk(unsigned int blk) {
	start_pos = blk * bit_size + base_pos;
	if (bit_size + start_pos > bits.length()) throw std::runtime_error("over flow");
}


}//namespace
