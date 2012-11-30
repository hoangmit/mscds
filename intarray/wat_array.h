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

#ifndef __WAVELET_ARRAY_H_
#define __WAVELET_ARRAY_H_

#include <vector>
#include <queue>
#include <stdint.h>
#include <iostream>
#include <list>
#include <cassert>
#include "bitarray/rank6p.h"
#include "archive.h"

namespace mscds {

class WatQuery {
public:
	static const uint64_t NOTFOUND = 0xFFFFFFFFFFFFFFFFULL;

	uint64_t access(uint64_t pos) const;
	uint64_t rank(uint64_t c, uint64_t pos) const;
	uint64_t select(uint64_t c, uint64_t r) const;
	uint64_t rankLessThan(uint64_t c, uint64_t pos) const;
	uint64_t rankMoreThan(uint64_t c, uint64_t pos) const;
	void rankAll(uint64_t c, uint64_t pos,
		uint64_t& rank,  uint64_t& rank_less_than, uint64_t& rank_more_than) const;
		
	uint64_t kthValue(uint64_t begin_pos, uint64_t end_pos, uint64_t k, uint64_t & pos) const;
	uint64_t maxValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const;
	uint64_t minValue(uint64_t begin_pos, uint64_t end_pos, uint64_t & pos) const;

	uint64_t length() const;

	WatQuery();
	~WatQuery();

	uint64_t count2d(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos) const;
	typedef bool (*ListCallback) (void * context, uint64_t c, uint64_t pos);
	/** return number in range [min_c, max_c) and [beg_pos, end_pos) */
	void list_each(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos, ListCallback cb, void* context) const;

	void load(IArchive& ar);
	void save(OArchive& ar) const;
	void clear();
	std::string to_str() const;

	const BitArray& bit_layer(unsigned int d);
private:
	std::vector<Rank6p> bit_arrays;
	uint64_t slength;
	uint64_t bitwidth;
	uint64_t max_val;

	uint64_t select_rec(uint64_t c, uint64_t r, size_t level, uint64_t beg_node, uint64_t end_node) const ;

	friend struct RecListEnv;
	friend class GridQuery;
	friend class WatBuilder;
};


class WatBuilder {
public:
	//static std::vector<uint64_t> convert(const std::vector<unsigned int>& list);
	void build(const std::vector<uint64_t>& list, WatQuery * out);
	void build(const std::vector<uint64_t>& list, OArchive & ar);
private:
};


class GridQuery {
private:
	const WatQuery* wt;
	std::vector<unsigned int> num_lst;
public:
	GridQuery(): wt(NULL) {}

	void process(const WatQuery* wt, const std::vector<unsigned int>& pos, const std::vector<unsigned int>& num, std::vector<unsigned int>  * result);
	void clear();
private:
	unsigned int poslen;
	std::vector<unsigned int> * results;
	struct Query2{
		unsigned int beg_node, end_node;
		unsigned int depth;
		unsigned int beg_plst, end_plst;

		Query2() {}
		Query2(unsigned int beg, unsigned int end, unsigned int d, 
			unsigned bl, unsigned el):
				beg_node(beg), end_node(end), depth(d),
				beg_plst(bl), end_plst(el) {}

		struct PosInfo {
			PosInfo() {}
			PosInfo(unsigned int _pos, unsigned int _rank_lt):
				pos(_pos), rank_lt(_rank_lt) {}

			unsigned int pos, rank_lt;
		};
		std::vector<PosInfo> qpos;
	};

	unsigned int list_partition(unsigned int depth, unsigned int beg_list, unsigned int end_list) const;
	void collect(const Query2& q);
	void expandQ(const Query2& q, std::deque<Query2>& output) ;
};

} //namespace


#endif // __WAVELET_ARRAY_H_
