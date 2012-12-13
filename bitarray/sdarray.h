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

#include "archive.h"
#include "bitarray.h"

#include <vector>
#include <sstream>
#include <stdint.h>
#include <iostream>
#include <stdexcept>



namespace mscds{

class SDArrayQuery;

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

	void build_bits(BitArray& ba, SDArrayQuery * out);
	/*
   * Build an index. This build should be called before prefixSum(), prefixSumLookup(), and find().
   */
	void build(OArchive& ar);
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
};


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

	/*
   * @ret vals_[0]+vals_[1]+...+vals_[pos-1] and set vals_[pos] to val
   */
	uint64_t lookup(uint64_t pos) const;

	/*
   * @ret Return ind s.t. prefixSum(ind) <= val < prefixSum(ind+1) or NOTFOUND if not exist
   */
	uint64_t find(uint64_t val) const; // upper_bound(val) - 1

	size_t length() const;

	void load(IArchive& ar);
	void save(OArchive& ar) const;
	void clear();

	std::string to_str(bool psum = false) const; 

private:
	static uint64_t selectWord(uint64_t x, uint64_t r);
	uint64_t selectBlock(uint64_t rank, uint64_t header) const;
	uint64_t rankBlock(uint64_t val, uint64_t header) const;
	uint64_t getLow(uint64_t begPos, uint64_t num, uint64_t width) const;
	uint64_t getBitI(uint64_t pos) const;
	uint64_t getBitsI(uint64_t pos, uint64_t num) const;
	BitArray Ltable_;
	BitArray B_;
	size_t size_;
	uint64_t sum_;
	friend class SDArrayBuilder;
};

class SDRankSelect {
public:
	SDRankSelect() {}
	~SDRankSelect() { clear(); }

	void build(const std::vector<uint64_t>& inc_pos) {
		clear();
		bool b = std::is_sorted(inc_pos.begin(), inc_pos.end());
		if (!b) throw std::logic_error("required sorted array");
		for (size_t i = 1; i < inc_pos.size(); i++) 
			if (inc_pos[i] == inc_pos[i-1]) throw std::logic_error("required non-duplicated elements");
		if (inc_pos.size() == 0) return;
		SDArrayBuilder bd;
		if (inc_pos[0] == 0) bd.add(0);
		else bd.add(inc_pos[0]);
		for (size_t i = 1; i < inc_pos.size(); i++) 
			bd.add(inc_pos[i] - inc_pos[i-1]);
		bd.build(&qs);
	}

	void build(BitArray& ba) {
		clear();
		SDArrayBuilder bd;
		uint64_t last = 0;
		for (size_t i = 0; i < ba.length(); i++)
			if (ba[i]) {
				bd.add(i-last);
				last = i;
			}
		bd.build(&qs);
	}

	uint64_t one_count() const {
		return qs.length();
	}

	uint64_t rank(uint64_t p) const {
		if (p == 0) return 0;
		uint64_t k = qs.find(p);
		if (k == SDArrayQuery::NOTFOUND)
			return qs.length();
		if (qs.prefixsum(k) != p) return k;
		else return k - 1;
	}

	uint64_t select(uint64_t r) const {
		assert(r < one_count());
		return qs.prefixsum(r+1);
	}

	void load(IArchive& ar) {
		qs.load(ar);
	}

	void save(OArchive& ar) const {
		qs.save(ar);
	}

	void clear() {
		qs.clear();
	}

	std::string to_str() const {
		std::ostringstream ss;
		ss << '{';
		if (qs.length() > 0) {
			ss << qs.prefixsum(1);
			for (size_t i = 2; i <= qs.length(); i++) 
				ss << ',' << qs.prefixsum(i);
		}
		ss << '}';
		return ss.str();
	}
public:
	SDArrayQuery qs;
};


}//namespace

#endif // SDARRAY_HPP__
