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

#ifndef SDARRAY_HPP__
#define SDARRAY_HPP__

#include "archive.h"
#include "bitarray.h"

#include <vector>
#include <stdint.h>
#include <iostream>



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



}//namespace

#endif // SDARRAY_HPP__
