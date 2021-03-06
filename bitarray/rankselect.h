#pragma once

#ifndef __RANK_SELECT_H_
#define __RANK_SELECT_H_

/** 
\file
Common interface of rank/select data structures

*/

#include <stdint.h>

namespace mscds {

/// Rank/Select data structure interface
class RankSelectInterface {
public:
	/// returns the number of 1-bits in [0..p) (from 0, until p, p is excluded)
	virtual uint64_t rank(uint64_t p) const = 0;
	virtual uint64_t select(uint64_t r) const = 0;
	virtual uint64_t selectzero(uint64_t r) const = 0;
	virtual bool access(uint64_t pos) const = 0;
	virtual uint64_t one_count() const = 0;
	virtual uint64_t length() const = 0;

	virtual bool bit(uint64_t p) const { return access(p); }
	virtual uint64_t rank(bool bit, uint64_t p) const { if (bit) return rank(p); else return rankzero(p); }
	virtual uint64_t rankzero(uint64_t p) const { return p - rank(p); }
	virtual uint64_t select(bool bit, uint64_t r) const { if (bit) return select(r); else return selectzero(r); }
	virtual uint64_t zero_count() { return length() - one_count(); }
};

}//namespace

#endif //__RANK_SELECT_H_