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

#pragma once

#ifndef SDARRAY_HPP__
#define SDARRAY_HPP__

#include "framework/archive.h"
#include "bitarray/bitarray.h"

#include <vector>
#include <sstream>
#include <stdint.h>
#include <iostream>
#include <stdexcept>

namespace mscds{

class SDArrayQuery;
class SDRankSelect;

class SDArrayBuilder {
	static const uint64_t BEGPOS_WIDTH;
	static const uint64_t BEGPOS_MASK;
	static const uint64_t BLOCK_SIZE;
public:
	SDArrayBuilder();
	~SDArrayBuilder();
	/*
   * Add new item to the last
   * val >= 0
   */
	void add(uint64_t val);

	void add_inc(uint64_t val);
	uint64_t current_sum();

	void build_bits(BitArray& ba, SDArrayQuery * out);
	/*
   * Build an index. This build should be called before prefixSum(), prefixSumLookup(), and find().
   */
	void build(OutArchive& ar);
	void build(SDArrayQuery * out);
	void clear();
private:
	void build_inc();

	void packHighs(uint64_t begPos, uint64_t width);
	void packLows(uint64_t begPos, uint64_t width);
	static uint64_t log2(uint64_t x);


	std::vector<uint64_t> Ltable_;
	std::vector<uint64_t> B_;
	size_t size_;

	std::vector<uint64_t> vals_;
	uint64_t sum_;
	uint64_t last;
};

/// Sadasan's SDArray
class SDArrayQuery {
	static const uint64_t BLOCK_SIZE;
public:
	const static uint64_t NOTFOUND = 0xFFFFFFFFFFFFFFFFULL;
	SDArrayQuery();
	~SDArrayQuery();
	/*
   * @ret vals_[0]+vals_[1]+...+vals_[pos-1]
   */
	uint64_t prefixsum(uint64_t pos) const;
	
	/** return a pair of rank(p) and prefixsum(rank(p)) */
	uint64_t rank2(uint64_t p, uint64_t& select) const;

	/*
   * @ret vals_[0]+vals_[1]+...+vals_[pos-1] and set vals_[pos] to val
   */
	uint64_t lookup(uint64_t pos) const;
	uint64_t lookup(uint64_t pos, uint64_t& prev_sum) const;

	/*
   * @ret Return ind s.t. prefixSum(ind) <= val < prefixSum(ind+1) or NOTFOUND if not exist
   */
	uint64_t rank(uint64_t val) const; // upper_bound(val) - 1
	uint64_t rank2(uint64_t val, uint64_t& select) const {
		uint64_t v = rank(val);
		select = prefixsum(v);
		return v;
	}

	size_t length() const;
	uint64_t total() const { return sum_; }

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();

	void dump_text(std::ostream& fo) const; /* len, then numbers */
	std::string to_str(bool psum = false) const; 

private:
	static uint64_t selectWord(uint64_t x, uint64_t r);
	uint64_t selectBlock(uint64_t rank, uint64_t header) const;
	uint64_t rankBlock(uint64_t val, uint64_t header) const;
	uint64_t getLow(uint64_t begPos, uint64_t num, uint64_t width) const;
	uint64_t getBitI(uint64_t pos) const;
	uint64_t getBitsI(uint64_t pos, uint64_t num) const;

	// return i such that A[i] < val <= val[i+1]
	uint64_t hint_find2(uint64_t val, uint64_t low, uint64_t high) const;
	uint64_t rankBlock2(uint64_t val, uint64_t header) const;

	BitArray Ltable_;
	BitArray B_;
	size_t size_;
	uint64_t sum_;
	friend class SDArrayBuilder;
	friend class SDRankSelect;
	friend struct SDAIIterator;
};

class SDRankSelect;

class SDRankSelectBuilder {
public:
	SDRankSelectBuilder() { last = 0; }
	void add(uint64_t delta) { assert(delta > 0); last += delta; add(last); }
	void add_inc(uint64_t v) {  assert(v >= last); vals.push_back(v); last = v; }
	void clear() { vals.clear(); }
	void build(SDRankSelect* out);
	void build(OutArchive& ar);
private:
	std::vector<uint64_t> vals;
	uint64_t last;
};

class SDRankSelect {
public:
	SDRankSelect() {}
	~SDRankSelect() { clear(); }

	void build(const std::vector<uint64_t>& inc_pos);
	void build(const std::vector<unsigned int>& inc_pos);
	void build(BitArray& ba);

	uint64_t one_count() const { return qs.length(); }

	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t r) const { assert(r < one_count()); return qs.prefixsum(r+1); }

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	void clear() { qs.clear(); rankhints.clear(); }
	std::string to_str() const;
private:
	void initrank();
	unsigned int ranklrate;
	SDArrayQuery qs;
	FixedWArray rankhints;
};

inline void SDRankSelectBuilder::build(OutArchive& ar) { SDRankSelect a; a.save(ar); }
inline void SDRankSelectBuilder::build(SDRankSelect* out) { out->build(vals);};

}//namespace

#endif // SDARRAY_HPP__
