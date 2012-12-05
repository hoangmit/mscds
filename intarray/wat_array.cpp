/* 
*  Copyright (c) 2010 Daisuke Okanohara
* 
*   Redistribution and use in source and binary forms, with or without
*   modification, are permitted provided that the following conditions
*   are met:
* 
*   1. Redistributions of source code must retain the above Copyright
*      notice, this list of conditions and the following disclaimer.
*
*   2. Redistributions in binary form must reproduce the above Copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*
*   3. Neither the name of the authors nor the names of its contributors
*      may be used to endorse or promote products derived from this
*      software without specific prior written permission.
*/

#include <queue>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include "wat_array.h"

#include "bitarray/bitop.h"


namespace mscds {

using namespace std;

static inline uint64_t getMSB(uint64_t x, uint64_t pos, uint64_t len) {
	return (x >> (len - (pos + 1))) & 1ULL;
}

static inline uint64_t prefixCode(uint64_t x, unsigned int len, unsigned int bit_num) {
	return x >> (bit_num - len);
}

void sortrun(unsigned int d, unsigned int bit_num, vector<uint64_t>& pos, vector<uint64_t>& runlen);

void WatBuilder::build(const vector<uint64_t>& list, WatQuery * out) {
	uint64_t alphabet_num = 0;
	for (size_t i = 0; i < list.size(); ++i){
		if (list[i] >= alphabet_num)
			alphabet_num = list[i];
	}

	uint64_t alphabet_bit_num_ = ceillog2(alphabet_num);
	//assert(Log2(alphabet_num) == msb_intr(alphabet_num - 1) + 1);

	uint64_t length = static_cast<uint64_t>(list.size());
	out->clear();
	out->max_val = alphabet_num;
	out->slength = length;
	out->bitwidth = alphabet_bit_num_;
	BitArray v = BitArray::create(length * alphabet_bit_num_);
	v.fillzero();
	
	vector<uint64_t> runlen, pos(list);
	runlen.push_back(length);
	for (unsigned int d = 0; d < alphabet_bit_num_; ++d) {
		for (unsigned int i = 0; i < length; ++i) 
			v.setbit(d*length + i, getMSB(pos[i], d, alphabet_bit_num_) != 0);
		if (d + 1 <  alphabet_bit_num_) {
			sortrun(d, alphabet_bit_num_, pos, runlen);
		}
	}
	Rank6pBuilder::build(v, &(out->bit_array));
}

void WatBuilder::build(const vector<uint64_t>& list, OArchive & ar) {
	WatQuery q;
	build(list, &q);
	q.save(ar);
}

void sortrun(unsigned int d, unsigned int bit_num, vector<uint64_t>& pos, vector<uint64_t>& runlen) {
	vector<uint64_t> nextrunlen, r0, r1;
	uint64_t i = 0;
	for (uint64_t r = 0; r < runlen.size(); ++r) {
		uint64_t len = runlen[r];
		if (len > 0) {
			uint64_t zero_cnt = 0;
			for (uint64_t j = 0; j < len; j++)
				if (getMSB(pos[i+j], d, bit_num) == 0) {
					zero_cnt++;
					r0.push_back(pos[i+j]);
				}else 
					r1.push_back(pos[i+j]);
			uint64_t one_cnt = len - zero_cnt;
			i += len;
			nextrunlen.push_back(zero_cnt);
			nextrunlen.push_back(one_cnt);
		}
	}
	i = 0;
	runlen.clear();
	uint64_t r0p = 0, r1p = 0;
	for (uint64_t k = 0; k < nextrunlen.size(); k += 2) {
		uint64_t zero_cnt = nextrunlen[k];
		uint64_t one_cnt = nextrunlen[k+1];
		for (uint64_t j = 0; j < zero_cnt; j++)
			pos[i+j] = r0[r0p + j];
		i += zero_cnt;
		r0p += zero_cnt;
		for (uint64_t j = 0; j < one_cnt; j++)
			pos[i+j] = r1[r1p + j];
		i += one_cnt;
		r1p += one_cnt;
	}
	r0.clear();
	r1.clear();
	runlen.swap(nextrunlen);
}


//--------------------------------------------------------------------------------------
#define SSTR( x ) dynamic_cast<std::ostringstream &>((std::ostringstream() <<  x )).str()

	void WatQuery::load(IArchive& ar) {
		clear();
		unsigned char ver = ar.loadclass("wavelet_tree");
		if (ver < 2) throw runtime_error("incompatible with version < 2");
		ar.var("length").load(slength);
		ar.var("bitwidth").load(bitwidth);
		ar.var("max_value").load(max_val);
		ar.var("bits");
		BitArray b;
		b.load(ar);
		ar.var("rank");
		bit_array.load(ar, b);
		ar.endclass();
	}

	void WatQuery::save(OArchive& ar) const {
		ar.startclass("wavelet_tree", 2);
		ar.var("length").save(slength);
		ar.var("bitwidth").save(bitwidth);
		ar.var("max_value").save(max_val);
		ar.var("bits");
		bit_array.getBitArray().save(ar);
		ar.var("rank");
		bit_array.save(ar);
		ar.endclass();
	}


	const BitArray& WatQuery::bit_layers() {
		return bit_array.getBitArray();
	}

	uint64_t WatQuery::access(uint64_t pos) const {
		if (pos >= slength) throw std::runtime_error("out of range");
		uint64_t st = 0;
		uint64_t en = slength;
		uint64_t c = 0;
		for (size_t i = 0; i < bitwidth; ++i){
			const Rank6p & ba = bit_array;
			const uint64_t boundary = st - ba.rankzero(st) + ba.rankzero(en);
			c <<= 1;
			if (ba.bit(st + pos)){
				pos = ba.rank(st + pos) - ba.rank(st);
				st = boundary + slength;
				en += slength;
				c |= 1ULL;
			} else {
				pos = ba.rankzero(st + pos) - ba.rankzero(st);
				st += slength;
				en = boundary + slength;
			}
		}
		return c;	 
	}

	uint64_t WatQuery::rank(uint64_t c, uint64_t pos) const{
		uint64_t rank_less_than = 0, rank_more_than = 0, rank = 0;
		rankAll(c, pos, rank, rank_less_than, rank_more_than);
		return rank;
	}

	uint64_t WatQuery::rankLessThan(uint64_t c, uint64_t pos) const{
		uint64_t rank_less_than = 0, rank_more_than = 0, rank = 0;
		rankAll(c, pos, rank, rank_less_than, rank_more_than);
		return rank_less_than;
	}

	uint64_t WatQuery::rankMoreThan(uint64_t c, uint64_t pos) const{
		uint64_t rank_less_than = 0, rank_more_than = 0, rank = 0;
		rankAll(c, pos, rank, rank_less_than, rank_more_than);
		return rank_more_than;
	}

	void WatQuery::rankAll(uint64_t c, uint64_t pos,
		uint64_t& rank,  uint64_t& rank_less_than, uint64_t& rank_more_than) const{
			if (c > max_val) {
				rank_less_than = pos;
				rank_more_than = 0;
				rank           = 0;
				return;
			}
			if (pos >= slength) {
				pos = slength;
			}
			uint64_t beg_node = 0;
			uint64_t end_node = slength;
			rank_less_than = 0;
			rank_more_than = 0;
			for (size_t i = 0; i < bitwidth && beg_node < end_node; ++i) {
				const Rank6p& ba = bit_array;
				const uint64_t beg_node_zero = ba.rankzero(beg_node);
				const uint64_t boundary = beg_node + ba.rankzero(end_node) - beg_node_zero;
				if (getMSB(c, i, bitwidth) == 0){
					rank_more_than += ba.rank(pos) - (beg_node - beg_node_zero);
					pos      = beg_node + ba.rankzero(pos) - beg_node_zero + slength;
					beg_node += slength;
					end_node = boundary + slength;
				} else {
					rank_less_than += ba.rankzero(pos) - beg_node_zero;
					pos      = boundary + ba.rank(pos) - (beg_node - beg_node_zero) + slength;
					beg_node = boundary + slength;
					end_node += slength;
				}
			}
			rank = pos - beg_node;
	}

	uint64_t WatQuery::select(uint64_t c, uint64_t r) const {
		assert(r < slength);
		return select_rec(c, r, 0, 0, slength);
	}

	uint64_t WatQuery::select_rec(uint64_t c, uint64_t r, size_t level, uint64_t beg_node, uint64_t end_node) const {
		if (r + beg_node >= end_node) return NOTFOUND; 
		if (level == bitwidth)
			return r;
		const Rank6p& ba = bit_array;
		const uint64_t beg_node_zero = ba.rankzero(beg_node);
		const uint64_t boundary = beg_node + ba.rankzero(end_node) - beg_node_zero;
		uint64_t rs = NOTFOUND;
		if (getMSB(c, level, bitwidth) == 0) {
			rs = select_rec(c, r, level + 1, beg_node + slength, boundary + slength);
			if (rs == NOTFOUND) return NOTFOUND;
			return ba.selectzero(beg_node_zero + rs) - beg_node;
		} else {
			rs = select_rec(c, r, level + 1, boundary + slength, end_node + slength);
			if (rs == NOTFOUND) return NOTFOUND;
			return ba.select((beg_node - beg_node_zero) + rs) - beg_node;
		}
	}

	uint64_t WatQuery::count2d(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos) const {
		if (min_c >= max_val) return 0;
		if (max_c <= min_c) return 0;
		if (end_pos > length() || beg_pos > end_pos) return 0;
		return 
			+ rankLessThan(max_c, end_pos)
			- rankLessThan(min_c, end_pos)
			- rankLessThan(max_c, beg_pos)
			+ rankLessThan(min_c, beg_pos);
	}

	inline bool CheckPrefix(uint64_t prefix, unsigned int depth, uint64_t min_c, uint64_t max_c, unsigned int bitwidth)  {
		if (prefixCode(min_c, depth, bitwidth) <= prefix && prefixCode(max_c-1, depth, bitwidth) >= prefix) return true;
		else return false;
	}

	struct RecListEnv {
		RecListEnv(const WatQuery* p): ptr(p) { bitwidth = ptr->bitwidth; }

		const WatQuery * ptr;
		unsigned int bitwidth;
		void * context;
		WatQuery::ListCallback cb;

		uint64_t min_c, max_c;
		unsigned int cur_level;

		struct NodeInfo {
			uint64_t beg_node, end_node;
			uint64_t beg_pos, end_pos;
			uint64_t prefix_char;

			uint64_t beg_node_zero, beg_node_one;
			unsigned int depth;
			bool bit;
			NodeInfo() {}
			NodeInfo(uint64_t beg_node, uint64_t end_node, uint64_t beg_pos, uint64_t end_pos, 
				uint64_t depth, uint64_t prefix_char, bool _bit) :
			  beg_node(beg_node), end_node(end_node), beg_pos(beg_pos), end_pos(end_pos), 
			  depth(depth), prefix_char(prefix_char), bit(_bit) {}
		};

		std::vector<NodeInfo> stack;
		void query(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos, WatQuery::ListCallback cb, void* context) {
			this->context = context;
			this->min_c = min_c;
			this->max_c = max_c;
			this->cb = cb;
			stack.resize(bitwidth+1);
			stack[0] = NodeInfo(0, ptr->slength, beg_pos, end_pos, 0, 0, false);
			expand_rec(0);
			stack.clear();
		}

		uint64_t tracepos(unsigned int r) {
			for (size_t i = bitwidth; i > 0; --i) {
				const Rank6p& ba = (ptr->bit_array);
				if (!stack[i].bit)
					r = ba.selectzero(stack[i - 1].beg_node_zero + r) - stack[i - 1].beg_node;
				else
					r = ba.select(stack[i - 1].beg_node_one + r) - stack[i - 1].beg_node;
			}
			return r;
		}

		void expand_rec(unsigned int level) {
			NodeInfo & cur = stack[level];
			if (cur.beg_node >= cur.end_node) return ;
			if (level == bitwidth) {
				for (uint64_t i = cur.beg_pos - cur.beg_node; i < cur.end_pos - cur.beg_node; ++i) {
					cb(context, cur.prefix_char, tracepos(i));
				}
				return ;
			}
			const Rank6p& ba = (ptr->bit_array);
			const uint64_t slength = ptr->slength;
			cur.beg_node_zero = ba.rankzero(cur.beg_node);
			cur.beg_node_one  = cur.beg_node - cur.beg_node_zero;
			const uint64_t beg_zero  = ba.rankzero(cur.beg_pos);
			const uint64_t end_zero  = ba.rankzero(cur.end_pos);
			const uint64_t beg_one   = cur.beg_pos - beg_zero;
			const uint64_t end_one   = cur.end_pos - end_zero;
			const uint64_t boundary  = cur.beg_node + ba.rankzero(cur.end_node) - cur.beg_node_zero;
			if (end_zero - beg_zero > 0){
				uint64_t next_prefix = cur.prefix_char << 1;
				if (CheckPrefix(next_prefix, cur.depth+1, min_c, max_c, bitwidth)) {
					stack[level+1] = NodeInfo(cur.beg_node + slength, 
						boundary + slength, 
						cur.beg_node + beg_zero - cur.beg_node_zero + slength, 
						cur.beg_node + end_zero - cur.beg_node_zero + slength, 
						cur.depth+1,
						next_prefix, false);
					expand_rec(level + 1);
				}
			}
			if (end_one - beg_one > 0){
				uint64_t next_prefix = (cur.prefix_char << 1) + 1;
				if (CheckPrefix(next_prefix, cur.depth+1, min_c, max_c, bitwidth)) {
					stack[level+1] = NodeInfo(boundary + slength, 
						cur.end_node + slength, 
						boundary + beg_one - cur.beg_node_one + slength, 
						boundary + end_one - cur.beg_node_one + slength, 
						cur.depth+1,
						next_prefix, true);
					expand_rec(level + 1);
				}
			}

		}
	};

	void WatQuery::list_each(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos, WatQuery::ListCallback cb, void* context) const {
		if (min_c >= max_c || beg_pos >= end_pos) return ;
		RecListEnv rc(this);
		rc.query(min_c, max_c, beg_pos, end_pos, cb, context);
	}

	uint64_t WatQuery::kthValue(uint64_t begin_pos, uint64_t end_pos, uint64_t k, uint64_t & pos) const {
		uint64_t val;
		if (end_pos > slength || begin_pos >= end_pos) {
			//pos = NOTFOUND;
			//val = NOTFOUND;
			assert(false);
			return NOTFOUND;
		}

		val = 0;
		uint64_t beg_node = 0;
		uint64_t end_node = slength;
		for (size_t i = 0; i < bitwidth; ++i) {
			const Rank6p& ba = (bit_array);
			const uint64_t beg_node_zero = ba.rankzero(beg_node);
			const uint64_t beg_zero  = ba.rankzero(begin_pos);
			const uint64_t end_zero  = ba.rankzero(end_pos);
			const uint64_t boundary  = beg_node + ba.rankzero(end_node) - beg_node_zero;
			if (end_zero - beg_zero > k) {
				beg_node += slength;
				end_node = boundary + slength;
				begin_pos = beg_node + beg_zero - beg_node_zero;
				end_pos   = beg_node + end_zero - beg_node_zero;
				val       = val << 1;
			} else {
				const uint64_t beg_node_one  = beg_node - beg_node_zero;
				beg_node  = boundary + slength;
				end_node += slength;
				begin_pos = beg_node + (begin_pos - beg_zero) - beg_node_one;
				end_pos   = beg_node + (end_pos - end_zero) - beg_node_one;
				val       = (val << 1) + 1;
				k -= end_zero - beg_zero;
			}
		}

		uint64_t rank = begin_pos - beg_node;
		pos = select(val, rank);
		return val;
	}

	uint64_t WatQuery::length() const {
		return slength;
	}

	uint64_t WatQuery::maxValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const {
		return kthValue(begin_pos, end_pos, end_pos - begin_pos - 1, pos);
	}

	uint64_t WatQuery::minValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const {
		return kthValue(begin_pos, end_pos, 0, pos);
	}


	WatQuery::WatQuery():slength(0) {}
	WatQuery::~WatQuery() { if (slength > 0) { clear(); slength = 0; } }

	std::string WatQuery::to_str() const {
		ostringstream ss;
		ss << '{';
		if (length() > 0)
			ss << access(0);
		for (unsigned int i = 1; i < length(); ++i) {
			ss << ',' << access(i);
		}
		ss << '}';
		return ss.str();
	}
	void WatQuery::clear() {
		slength = 0;
		bitwidth = 0;
		max_val = 0;
		bit_array.clear();
	}

	//----------------------------------------------------------------------------

	void GridQuery::process(WatQuery const * wt, const std::vector<unsigned int>& pos, const std::vector<unsigned int>& num, std::vector<unsigned int> * result) {
		this->results = result;
		this->wt = wt;
		this->poslen = pos.size();
		assert(wt->length() < (1ULL << 32));
		num_lst.resize(num.size());
		std::copy(num.begin(), num.end(), num_lst.begin());
		std::sort(num_lst.begin(), num_lst.end());
		//std::sort(pos.begin(), pos.end());
		assert(pos.back() <= wt->length());
		//assert(num_lst.back() <= wt->alphabet_num());
		assert(num_lst.back() < 1ULL << (wt->bitwidth));
		Query2 q;
		q.beg_node = 0;
		q.end_node = wt->length();
		q.depth = 0;
		q.beg_plst = 0;
		q.end_plst = num_lst.size();
		//assert(wt->bit_array.size() == wt->bitwidth);
		for (auto it = pos.begin(); it != pos.end(); it++) 
			q.qpos.push_back(Query2::PosInfo(*it, 0));

		//results->resize(num.size());
		//for (unsigned int i = 0; i < results->size(); i++)
		//	(*results)[i].resize(pos.size());
		result->resize(num.size() * pos.size());

		std::deque<Query2> cur, next;
		cur.push_back(q);
		next.clear();
		for (unsigned int i = 0; i < wt->bitwidth; ++i) {
			for (auto it = cur.begin(); it != cur.end(); ++it)
				expandQ(*it, next);
			cur.swap(next);
			next.clear();
		}
		for (auto it = cur.begin(); it != cur.end(); ++it)
			collect(*it);

		num_lst.clear();
		this->wt = NULL;
	}

	void GridQuery::clear() {
		results->clear();
	}

	void GridQuery::collect(const Query2& q) {
		assert(q.qpos.size() == poslen);
		for (unsigned int i = q.beg_plst; i < q.end_plst; ++i) {
			unsigned int j = 0, st = i*poslen;
			for (auto it = q.qpos.begin(); it != q.qpos.end(); ++it) {
				(*results)[st + j] = it->rank_lt;
				j++;
			}
		}
	}

	unsigned int GridQuery::list_partition(unsigned int depth, unsigned int beg_list, unsigned int end_list) const {
		unsigned int count, step, it;
		unsigned int first = beg_list;
		count = end_list - beg_list;
		while (count > 0) {
			it = first; 
			step = count / 2; 
			it += step;
			if (getMSB(num_lst[it],depth,wt->bitwidth) == 0) { // !(value < num_lst[it]) // bitzero
				first = ++it;
				count -= step + 1;
			} else count = step;
		}
		return first;
	}

	void GridQuery::expandQ(const Query2& q, std::deque<Query2>& output) {
		if (q.beg_node >= q.end_node) {
			collect(q);
			return ;
		}
		const Rank6p& ba = wt->bit_array;
		uint64_t beg_node_zero = ba.rankzero(q.beg_node);
		uint64_t boundary = q.beg_node + ba.rankzero(q.end_node) - beg_node_zero;
		unsigned int list_boundary = list_partition(q.depth, q.beg_plst, q.end_plst);
		if (list_boundary > q.beg_plst){
			output.push_back(Query2(q.beg_node + wt->slength, boundary + wt->slength, q.depth + 1, 
				q.beg_plst, list_boundary));
			Query2 & zq = output.back();
			uint64_t lastp = ~0ull;
			unsigned int lastnpx = 0;
			zq.qpos.reserve(q.qpos.size());
			for (auto it = q.qpos.begin(); it != q.qpos.end(); ++it) {
				if (it->pos != lastp) {
					unsigned int npx = q.beg_node + ba.rankzero(it->pos) - beg_node_zero + wt->slength;
					zq.qpos.push_back(Query2::PosInfo(npx, it->rank_lt));
					lastp = it->pos;
					lastnpx = npx;
				}else
					zq.qpos.push_back(Query2::PosInfo(lastnpx, it->rank_lt));
			}
		}
		if (list_boundary < q.end_plst) {
			output.push_back(Query2(boundary + wt->slength, q.end_node + wt->slength, q.depth + 1, 
				list_boundary, q.end_plst));
			Query2 & nq = output.back();
			uint64_t lastp = ~0ull;
			nq.qpos.reserve(q.qpos.size());
			unsigned int lastnpx = 0, lastrlt = 0;
			for (auto it = q.qpos.begin(); it != q.qpos.end(); ++it) {
				if (it->pos != lastp) {
					unsigned int rleq = ba.rankzero(it->pos) - beg_node_zero;
					unsigned int npx = boundary + ba.rank(it->pos) - (q.beg_node - beg_node_zero) + wt->slength;
					nq.qpos.push_back(Query2::PosInfo(npx, rleq + it->rank_lt));
					lastp = it->pos;
					lastnpx = npx;
					lastrlt = rleq;
				}else
					nq.qpos.push_back(Query2::PosInfo(lastnpx, lastrlt + it->rank_lt));
			}
		}
	}

}//namespace
