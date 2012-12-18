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

#include <cassert>
#include "sdarray.h"
#include "bitop.h"
#include <cstring>
#include <stdexcept>
#include <sstream>
#include "utils/debug.h"

using namespace std;

namespace mscds {

const uint64_t SDArrayBuilder::BLOCK_SIZE = 64;
const uint64_t SDArrayQuery::BLOCK_SIZE = 64;

uint64_t SDArrayQuery::selectWord(uint64_t x, uint64_t r){
	assert(r > 0);
	return selectword(x,r-1);
}


uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num){
	return (x >> beg) & ((1ULL << num) - 1);
}

SDArrayBuilder::SDArrayBuilder() : size_(0), sum_(0) {}

SDArrayBuilder::~SDArrayBuilder(){}


void SDArrayBuilder::add(uint64_t val){
	vals_.push_back(val);
	size_++;
	if (vals_.size() == BLOCK_SIZE) {
		build_inc();
	}
}

uint64_t SDArrayBuilder::log2(uint64_t x) {
	uint64_t r = 0;
	while (x >> r){
		r++;
	}
	return r;
} 

void SDArrayBuilder::build_inc(){
	assert(vals_.size() <= BLOCK_SIZE);
	if (vals_.size() == 0) return;

	for (size_t i = 1; i < vals_.size(); ++i){
		vals_[i] += vals_[i-1];
	}

	uint64_t begPos  = B_.size();

	Ltable_.push_back(sum_);
	sum_ += vals_.back();

	// header
	// |-- begPos   (48) --|
	// |-- allZero  ( 1) --|
	// |-- width    ( 7) --|
	// |-- firstSum ( 8) --|
	assert(begPos < 1ULL << 48);
	uint64_t header = (uint64_t)begPos; // use first 48 bit only

	if (vals_.back() == 0){
		header |= (1ULL << 48);
	} else {
		uint64_t width = log2(vals_.back() / vals_.size());
		assert(width < (1ULL << 7));

		// All zero special case
		B_.resize(begPos + 2 + (vals_.size() * width + 63) / 64);
		packHighs(begPos, width);
		packLows(begPos, width);

		header |= (width << 49);
		uint64_t firstSum_ = popcnt(B_[begPos]);
		assert(firstSum_ < (1ULL << 8));

		header |= firstSum_ << 56;
	}
	Ltable_.push_back(header);
	vals_.clear();
}

void SDArrayBuilder::packHighs(uint64_t begPos, uint64_t width){
	for (size_t i = 0; i < vals_.size(); ++i){
		uint64_t pos    = (vals_[i] >> width) + i;
		B_[begPos + (pos / BLOCK_SIZE)] |= (1ULL << (pos % BLOCK_SIZE));
	}
}

void SDArrayBuilder::packLows(uint64_t begPos, uint64_t width){
	if (width == 0) return;
	begPos += 2;
	uint64_t mask   = (1ULL << width) - 1;
	for (size_t i = 0; i < vals_.size(); ++i){
		uint64_t val    = vals_[i] & mask;
		uint64_t pos    = i * width;
		uint64_t bpos   = pos / BLOCK_SIZE;
		uint64_t offset = pos % BLOCK_SIZE;
		B_[begPos + bpos] |= val << offset;
		if (offset + width > BLOCK_SIZE){
			B_[begPos + bpos + 1] |= (val >> (BLOCK_SIZE - offset));
		}
	}
}

void SDArrayBuilder::clear(){
	Ltable_.clear();
	B_.clear();
	vals_.clear();
	size_ = 0;
	sum_  = 0;
}

void SDArrayBuilder::build(OArchive& ar){
	build_inc();
	ar.startclass("sdarray", 1);
	ar.var("size").save(size_);
	ar.var("sum").save(sum_);
	BitArray b(&(B_[0]), B_.size() * 64);
	b.save(ar.var("bits"));
	BitArray l(&(Ltable_[0]), Ltable_.size() * 64);
	l.save(ar.var("table"));
	ar.endclass();
	clear();
}

void SDArrayBuilder::build(SDArrayQuery * out) {
	build_inc();
	out->clear();
	out->size_ = this->size_;
	out->sum_ = this->sum_;
	if (B_.size() > 0)
		out->B_ = BitArray::create(&(B_[0]), B_.size() * 64);
	if (Ltable_.size() > 0)
		out->Ltable_ = BitArray::create(&(Ltable_[0]), Ltable_.size() * 64);
	clear();
}

//------------------------------------------------------------------------------

void SDArrayQuery::save(OArchive& ar) const {
	ar.startclass("sdarray", 1);
	ar.var("size").save(size_);
	ar.var("sum").save(sum_);
	B_.save(ar.var("bits"));
	Ltable_.save(ar.var("table"));
	ar.endclass();
}

void SDArrayQuery::load(IArchive& ar) {
	clear();
	ar.loadclass("sdarray");
	ar.var("size").load(size_);
	ar.var("sum").load(sum_);
	B_.load(ar.var("bits"));
	Ltable_.load(ar.var("table"));
	ar.endclass();
}

void SDArrayQuery::clear() {
	B_.clear();
	Ltable_.clear();
	size_ = 0;
	sum_ = 0;
}

SDArrayQuery::SDArrayQuery():size_(0), sum_(0) {}
SDArrayQuery::~SDArrayQuery() {clear();}

uint64_t SDArrayQuery::prefixsum(const uint64_t pos) const {
	if (pos >= size_) return sum_;
	uint64_t bpos   = pos / BLOCK_SIZE;
	uint64_t offset = pos % BLOCK_SIZE;
	uint64_t sum    = Ltable_.word(bpos * 2);
	if (offset == 0) {
		return sum;
	}
	return sum + selectBlock(offset, Ltable_.word(bpos * 2 + 1));
}

uint64_t SDArrayQuery::lookup(const uint64_t pos) const {
	uint64_t bpos   = pos / BLOCK_SIZE;
	uint64_t offset = pos % BLOCK_SIZE;
	//uint64_t sum    = Ltable_.word(bpos * 2);
	uint64_t prev   = 0;
	if (offset == 0) {
		prev = 0;
	} else {
		prev = selectBlock(offset, Ltable_.word(bpos * 2 + 1));
	}
	uint64_t cur = selectBlock(offset+1, Ltable_.word(bpos * 2 + 1));
	return cur - prev;
	//val = ;
	//return sum + prev;
}


uint64_t SDArrayQuery::find(const uint64_t val) const {
	if (sum_ < val) {
		//cout << "come0" << endl;
		return NOTFOUND;
	}
	uint64_t low  = 0;
	uint64_t high = Ltable_.word_count() / 2;
	while (low < high){
		uint64_t mid = low + (high - low)/2;
		if (val < Ltable_.word(mid*2)){
			high = mid;
		} else {
			low = mid+1;
		}
	}
	assert(low*2 <= Ltable_.word_count());
	if (low == 0) return 0;
	uint64_t bpos = low-1;
	assert(Ltable_.word(bpos*2) <= val);
	assert(low*2 == Ltable_.word_count() || val < Ltable_.word(low*2));

	return bpos * BLOCK_SIZE + rankBlock(val - Ltable_.word(bpos*2), Ltable_.word(bpos*2+1));
} 

uint64_t SDArrayQuery::hint_find(uint64_t val, uint64_t low, uint64_t high) const {
	//uint64_t high = Ltable_.word_count() / 2;
	while (low < high){
		uint64_t mid = low + (high - low)/2;
		if (val < Ltable_.word(mid*2)){
			high = mid;
		} else {
			low = mid+1;
		}
	}
	assert(low*2 <= Ltable_.word_count());
	if (low == 0) return 0;
	uint64_t bpos = low-1;
	assert(Ltable_.word(bpos*2) <= val);
	assert(low*2 == Ltable_.word_count() || val < Ltable_.word(low*2));
	uint64_t vn = rankBlock2(val - Ltable_.word(bpos*2), Ltable_.word(bpos*2+1));;
	if (vn > 0)
		return bpos * BLOCK_SIZE + vn;
	else {
		if (Ltable_.word(bpos*2) == val) return bpos * BLOCK_SIZE - 1;
		else return bpos * BLOCK_SIZE;
	}
} 

uint64_t SDArrayQuery::rankBlock2(const uint64_t val, uint64_t header) const {
	if (getBits(header, 48, 1)){
		// all zero
		return BLOCK_SIZE-1; // <
	}
	uint64_t begPos      = getBits(header,  0, 48);
	uint64_t width       = getBits(header, 49,  7);
	uint64_t firstOneSum = getBits(header, 56,  8);

	uint64_t high = val >> width;
	uint64_t low  = getBits(val,  0, width);

	uint64_t firstZeroSum = BLOCK_SIZE - firstOneSum;
	uint64_t valNum = 0;
	uint64_t highPos = begPos * BLOCK_SIZE;
	if (high > firstZeroSum){
		valNum += firstOneSum;
		high -= firstZeroSum;
		highPos += BLOCK_SIZE;
	}
	if (high > 0){
		uint64_t skipNum = selectWord(~B_.word(highPos / BLOCK_SIZE), high)+ 1;
		highPos += skipNum;
		assert(skipNum >= high);
		valNum += skipNum - high;
	}

	for ( ; ;  highPos++, valNum++){
		if (highPos >= (begPos + 2) * BLOCK_SIZE){
			return valNum; // <
		}
		if (!getBitI(highPos)){
			return valNum; // <
		}
		uint64_t cur = getLow(begPos, valNum, width);

		if (cur == low) {
			return valNum; // =
		} else if (low < cur){
			return valNum; //
		}
	}
	return valNum;
}



size_t SDArrayQuery::length() const {
	return size_;
}

uint64_t SDArrayQuery::selectBlock(const uint64_t offset, const uint64_t header) const {
	if (getBits(header, 48, 1)){
		// all zero
		return 0;
	}
	uint64_t begPos   = getBits(header,  0, 48);
	uint64_t width    = getBits(header, 49,  7);
	uint64_t firstSum = getBits(header, 56,  8);

	uint64_t high = 0;
	if (offset <= firstSum) {
		high = (selectWord(B_.word(begPos), offset) + 1 - offset) << width;
	} else {
		high = (selectWord(B_.word(begPos+1), offset - firstSum) + 1 - offset + BLOCK_SIZE) << width;
	}

	return high + getLow(begPos, offset-1, width);
}

uint64_t SDArrayQuery::rankBlock(const uint64_t val, uint64_t header) const {
	if (getBits(header, 48, 1)){
		// all zero
		return BLOCK_SIZE-1;
	}
	uint64_t begPos      = getBits(header,  0, 48);
	uint64_t width       = getBits(header, 49,  7);
	uint64_t firstOneSum = getBits(header, 56,  8);

	uint64_t high = val >> width;
	uint64_t low  = getBits(val,  0, width);

	uint64_t firstZeroSum = BLOCK_SIZE - firstOneSum;
	uint64_t valNum = 0;
	uint64_t highPos = begPos * BLOCK_SIZE;
	if (high > firstZeroSum){
		valNum += firstOneSum;
		high -= firstZeroSum;
		highPos += BLOCK_SIZE;
	}
	if (high > 0){
		uint64_t skipNum = selectWord(~B_.word(highPos / BLOCK_SIZE), high)+ 1;
		highPos += skipNum;
		assert(skipNum >= high);
		valNum += skipNum - high;
	}

	for ( ; ;  highPos++, valNum++){
		if (highPos >= (begPos + 2) * BLOCK_SIZE){
			return valNum;
		}
		if (!getBitI(highPos)){
			return valNum;
		}
		uint64_t cur = getLow(begPos, valNum, width);

		if (cur == low) {
			return valNum + 1;
		} else if (low < cur){
			return valNum;
		}
	}
	return valNum;
}

uint64_t SDArrayQuery::getLow(uint64_t begPos, uint64_t num, uint64_t width) const{
	return getBitsI((begPos + 2) * BLOCK_SIZE + num * width, width);
}

uint64_t SDArrayQuery::getBitI(const uint64_t pos) const{
	return (B_.word(pos / BLOCK_SIZE) >> (pos % BLOCK_SIZE)) & 1ULL;
}

uint64_t SDArrayQuery::getBitsI(const uint64_t pos, const uint64_t num) const {
	uint64_t bpos   = pos / BLOCK_SIZE;
	uint64_t offset = pos % BLOCK_SIZE;
	uint64_t mask   = (1ULL << num) - 1;
	if (offset + num <= BLOCK_SIZE){
		return (B_.word(bpos) >> (pos % 64)) & mask;
	} else {
		return ((B_.word(bpos) >> (pos % 64)) + (B_.word(bpos + 1) << (BLOCK_SIZE - offset))) & mask;
	}
}

std::string SDArrayQuery::to_str(bool psum) const {
	ostringstream ss;
	if (psum) {
		ss << '<';
		if (length() > 0)
			ss << prefixsum(1);
		for (unsigned int i = 2; i <= length(); ++i) {
			ss << ',' << prefixsum(i);
		}
		ss << '>';
	}else {
		ss << '{';
		if (length() > 0)
			ss << lookup(0);
		for (unsigned int i = 1; i < length(); ++i) {
			ss << ',' << lookup(i);
		}
		ss << '}';
	}
	return ss.str();
}
//----------------------------------------------------------------------------
void SDRankSelect::build(const std::vector<uint64_t>& inc_pos) {
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
	initrank();
}

void SDRankSelect::build(const std::vector<unsigned int>& inc_pos) {
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
	initrank();
}

void SDRankSelect::build(BitArray& ba) {
	clear();
	SDArrayBuilder bd;
	uint64_t last = 0;
	for (size_t i = 0; i < ba.length(); i++)
		if (ba[i]) {
			bd.add(i-last);
			last = i;
		}
	bd.build(&qs);
	initrank();
}

struct SDAIIterator {
	const SDArrayQuery& q;
	uint64_t p;
	SDAIIterator(const SDArrayQuery& _q): q(_q), p(0) {}
	uint64_t operator*() const { return q.Ltable_.word(2*p); }
	void operator++() { ++p; }
};

void SDRankSelect::initrank() {
	if (qs.length() == 0) return;
	ranklrate = ceillog2(qs.total() / qs.length() + 1) + 7;
	SDAIIterator it(qs);
	rankhints = bsearch_hints(it, qs.Ltable_.word_count() / 2, qs.total(), ranklrate);
}

uint64_t SDRankSelect::rank(uint64_t p) const {
	if (p == 0) return 0;
	if (p > qs.total()) return qs.length();
	uint64_t i = rankhints[p>>ranklrate], j = rankhints[(p>>ranklrate)+1];

	/*uint64_t kt = qs.find(p);
	kt = (qs.prefixsum(kt) != p) ? kt : kt - 1;*/

	uint64_t k = qs.hint_find(p, i, j);
	//assert(k == kt);

	return k;
}

void SDRankSelect::load(IArchive& ar) {
	ar.loadclass("sd_rank_select");
	qs.load(ar);
	ar.load(ranklrate);
	rankhints.load(ar);
	ar.endclass();
}

void SDRankSelect::save(OArchive& ar) const {
	ar.startclass("sd_rank_select", 1);
	qs.save(ar);
	ar.save(ranklrate);
	rankhints.save(ar);
	ar.endclass();
}

std::string SDRankSelect::to_str() const {
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

} //namespace
