#pragma once

#ifndef __WAVELET_ARRAY_H_
#define __WAVELET_ARRAY_H_

#include <vector>
#include <queue>
#include <stdint.h>
#include <iostream>
#include <list>
#include <cassert>
#include "bitarray/rank6p.h"
#include "framework/archive.h"

namespace mscds {


template<typename>
struct RecListEnv;

template<typename>
class WatBuilderGen;

template<typename RankSelect = Rank6p> 
class WatQueryGen {
public:
	typedef RankSelect RankSelectTp;
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
	uint64_t operator[](uint64_t pos) const { return access(pos); }

	uint64_t length() const;

	WatQueryGen();
	~WatQueryGen();

	uint64_t count2d(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos) const;
	typedef bool (*ListCallback) (void * context, uint64_t c, uint64_t pos);
	/** return the numbers in range [min_c, max_c) and [beg_pos, end_pos) */
	void list_each(uint64_t min_c, uint64_t max_c, uint64_t beg_pos, uint64_t end_pos, ListCallback cb, void* context) const;

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	std::string to_str() const;

	const RankSelect& bit_layers();
	typedef class WatBuilderGen<RankSelect> BuilderTp;
private:
	uint64_t slength;
	uint64_t bitwidth;
	uint64_t max_val;
	RankSelect bit_array;

	uint64_t select_rec(uint64_t c, uint64_t r, size_t level, uint64_t beg_node, uint64_t end_node) const ;


	template<typename>
	friend struct RecListEnv;
	template <typename>
	friend class GridQueryGen;
	template <typename>
	friend class WatBuilderGen;
};

template<typename RankSelect = Rank6p> 
class WatBuilderGen {
public:
	//static std::vector<uint64_t> convert(const std::vector<unsigned int>& list);
	static void build(const std::vector<uint64_t>& list, WatQueryGen<RankSelect> * out);
	static void build(const std::vector<uint64_t>& list, OutArchive & ar);
private:
};


template<typename WavTree>
class GridQueryGen {
private:
	const WavTree* wt;
	std::vector<unsigned int> num_lst;
public:
	GridQueryGen(): wt(NULL) {}

	void process(const WavTree* wt, const std::vector<unsigned int>& pos, const std::vector<unsigned int>& num, std::vector<unsigned int>  * result);
	void clear();
private:
	uint64_t poslen;
	std::vector<unsigned int> * results;
	struct Query2 {
		uint64_t beg_node, end_node;
		unsigned int depth;
		unsigned int beg_plst, end_plst;

		Query2() {}
		Query2(uint64_t beg, uint64_t end, unsigned int d, 
			unsigned int bl, unsigned int el):
				beg_node(beg), end_node(end), depth(d),
				beg_plst(bl), end_plst(el) {}

		struct PosInfo {
			PosInfo() {}
			PosInfo(uint64_t _pos, uint64_t _rank_lt):
				pos(_pos), rank_lt(_rank_lt) {}
			uint64_t pos, rank_lt;
		};
		std::vector<PosInfo> qpos;
	};

	unsigned int list_partition(unsigned int depth, unsigned int beg_list, unsigned int end_list) const;
	void collect(const Query2& q);
	void expandQ(const Query2& q, std::deque<Query2>& output) ;
};

typedef WatQueryGen<Rank6p> WatQuery;
typedef WatBuilderGen<Rank6p> WatBuilder;
typedef GridQueryGen<WatQuery> GridQuery;

} //namespace


#endif // __WAVELET_ARRAY_H_

#include "wat_array.hxx"
