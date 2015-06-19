#pragma once

/** \file

Alternative implementation of sdarray with bigger block.

*/

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"
#include "intarray.h"
#include "bitarray/rankselect.h"

#include "sdarray_interface.h"

#include <iostream>


namespace mscds {

class SDArraySml;

/// Builder class for SDArraySml
class SDArraySmlBuilder {
public:
	SDArraySmlBuilder();

	void add(uint64_t val);
	/** add increasing values */
	void add_inc(uint64_t pos);

	void build(SDArraySml* out);
	void build(OutArchive& ar);
	void clear();

	uint64_t current_sum();

	static const uint64_t BLKSIZE;
	static const uint16_t SUBB_PER_BLK;
	typedef SDArraySml QueryTp;

private:
	void packHighs(uint64_t begPos, uint64_t width);
	void packLows(uint64_t begPos, uint64_t width);

	void build_blk();
	std::vector<uint64_t> vals;
	OBitStream bits;
	std::vector<uint64_t> table;
	uint64_t cnt, p_sum, pslast;
};

/**
SDArray is an array of non-negative integers <math>A[1..m]</math>.
It provides the following operations:
*  access any element <math>A[i]</math> in constant time
*  compute <math>prefix\_sum(i)= \sum_{k=1}^{i-1} A[k]</math> in constant time. (note that <math>prefix\_sum(0)=0</math>)
*  compute <math>rank(p)</math> returns the minimal index i such that <math>prefix\_sum(i) \geq p</math> in logarithmic time.

Let <math>n=\sum_{k=1}^{m}A[i]</math>. The SDArray should uses about <math>m \log_2 (n/m) + 2m + o(n)</math> bits.
*/

class SDArraySml : public SDArrayInterface {
public:
	SDArraySml() { clear(); }
	~SDArraySml() { clear();  }
	/** returns the value of A[i] */
	uint64_t lookup(uint64_t i) const;

	/** returns the value of A[i] and  prev_sum=prefix_sum(i) */
	uint64_t lookup(uint64_t i, uint64_t& prev_sum) const;

	/** return the value of prefix_sum(i) */
	uint64_t prefixsum(size_t i) const;

	/** return the value of rank(p) */
	uint64_t rank(uint64_t p) const;

	/** return a pair of rank(p) and prefixsum(rank(p)) */
	uint64_t rank2(uint64_t p, uint64_t& select) const;

	/** clear the array (length becomes 0) */
	void clear();

	/** save and load functions */
	void save(OutArchive& ar) const;
	void load(InpArchive& ar);

	/** to string for debug */
	std::string to_str(bool psum) const;

	/** returns the sum of all the elements in the array */
	uint64_t total() const { return sum; }

	/** counts the number of elements in the array */
	uint64_t length() const { return len; }

	void dump_text(std::ostream& fo) const;

	typedef SDArraySmlBuilder BuilderTp;

	class PSEnum: public EnumeratorInt<uint64_t> {
	public:
		PSEnum(const PSEnum& o): basesum(o.basesum), ptr(o.ptr), hiptr(o.hiptr), 
			loptr(o.loptr), baseptr(o.baseptr), idx(o.idx), blkwidth(o.blkwidth) {}
		bool hasNext() const;
		uint64_t next();
		PSEnum(): ptr(NULL) {}
	private:
		PSEnum(const SDArraySml * p, uint64_t blk);
		void moveblk(uint64_t blk);

		uint64_t basesum;
		uint64_t hiptr, loptr;
		uint64_t baseptr, idx;
		uint16_t blkwidth;

		const SDArraySml * ptr;
		friend class SDArraySml;
	};

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		Enum(const Enum& o): e(o.e), last(o.last) {}
		bool hasNext() const { return e.hasNext(); }
		uint64_t next() { uint64_t v = e.next(); uint64_t d = v - last; last = v; return d; }
	private:
		//Enum(const PSEnum& _e, uint64_t _last): e(_e), last(_last) {}
		PSEnum e;
		uint64_t last;
		friend class SDArraySml;
	};
	void getEnum(size_t idx, Enum * e) const;
	void getPSEnum(size_t idx, PSEnum * e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	uint64_t rank(uint64_t val, uint64_t lo, uint64_t hi) const;

	uint64_t select_hi(uint64_t hints, uint64_t start, uint32_t p) const;

	uint64_t select_zerohi(uint64_t hints, uint64_t start, uint32_t p) const;
	uint64_t rankBlk(uint64_t blk, uint64_t val) const;
	
	BitArray bits;
	BitArray table;
	uint64_t len, sum;
	
	friend class SDArraySmlBuilder;
	static const uint64_t BLKSIZE;
	static const uint64_t SUBB_SIZE;

	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
	friend struct SDASIIterator;
	friend class SDRankSelectSml;
};

class SDRankSelectSml;

class SDRankSelectBuilderSml  {
public:
	SDRankSelectBuilderSml(): last(0) { }
	void add(uint64_t delta) { assert(delta > 0); last += delta; add_inc(last); }
	void add_inc(uint64_t pos) { assert(pos >= last); vals.push_back(pos); last = pos; }
	void clear() { vals.clear(); last = 0; }
	void build(SDRankSelectSml* out);
	void build(OutArchive& ar);
private:
	std::vector<uint64_t> vals;
	uint64_t last;
};


/// Rank Select interface of SDArraySml
class SDRankSelectSml: public RankSelectInterface {
public:
	SDRankSelectSml() {}
	~SDRankSelectSml() { clear(); }

	void build(const std::vector<uint64_t>& inc_pos);
	void build(const std::vector<unsigned int>& inc_pos);
	void build(BitArray& ba);

	uint64_t one_count() const { return qs.length(); }
	uint64_t length() const { return select(one_count() - 1); }

	bool bit(uint64_t p) const;
	uint64_t selectzero(uint64_t r) const { throw "not implemented"; return 0; }
	bool access(uint64_t pos) const { return bit(pos); }

	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t r) const { assert(r < one_count()); return qs.prefixsum(r+1); }
	
	/** return the value of prefix_sum(i) */
	uint64_t prefixsum(size_t i) const { return qs.prefixsum(i); }

	uint64_t total() const { return qs.total(); }

	// gap between p-th position of one and (p-1)-th position 
	const SDArraySml& sdarray() const { return qs; }

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	void clear() { qs.clear(); rankhints.clear(); }
	std::string to_str() const;
	struct DEnum: public EnumeratorInt<uint64_t> {
	public:
		DEnum() {}
		DEnum(const DEnum& o): re(o.re){}
		bool hasNext() const { return re.hasNext(); }
		uint64_t next() { return re.next(); }
	private:
		SDArraySml::Enum re;
		friend class SDRankSelectSml;
	};

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		Enum(const Enum& o): re(o.re){}
		bool hasNext() const { return re.hasNext(); }
		uint64_t next() { return re.next(); }
	private:
		SDArraySml::PSEnum re;
		friend class SDRankSelectSml;
	};

	void getDisEnum(size_t idx, DEnum * e) const { return qs.getEnum(idx, &(e->re));}
	void getEnum(size_t idx, Enum * e) const { return qs.getPSEnum(idx, &(e->re));}
	
	const SDArraySml& getArray() const { return qs; }
	void inspect(const std::string&, std::ostream&) const {}
private:
	SDArraySml qs;
	void initrank();
	unsigned int ranklrate;
	FixedWArray rankhints;
};

inline void SDRankSelectBuilderSml::build(OutArchive& ar) { SDRankSelectSml a; a.save(ar); }

inline void SDRankSelectBuilderSml::build(SDRankSelectSml* out) { out->build(vals);};


}//namespace
