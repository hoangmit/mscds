#pragma once

#ifndef __RMQ_SUPER_CARTISIAN_TREE_H_
#define __RMQ_SUPER_CARTISIAN_TREE_H_

/** \file

Data structure for Range Minimum/Maximum Query.

based on super cartisian tree.

*/

#include "BP_bits.h"
#include <cassert>
#include <stack>

namespace mscds {



/**
Implement supercartisian tree building procedure from
E. Ohlebusch and S. Gog. A Compressed Enhanced Suffix Array Supporting Fast String Matching. SPIRE 2009.
 */
template<typename RandomAccessIterator>
BitArray build_supercartisian_tree(bool minimum_tree, RandomAccessIterator first, RandomAccessIterator last);

/// succinct RMQ data structure
class RMQ_sct {
	BP_aux bp;
public:
	template<typename T>
	void build(const std::vector<T>& arr, bool minstr = true, unsigned int blksize = 256) {
		BitArray b = build_supercartisian_tree(minstr, arr.begin(), arr.end());
		bp.build(b, blksize);
	}
	void build(BitArray& b, unsigned int blksize = 256) {
		bp.build(b, blksize);
	}

	size_t m_idx(size_t st, size_t ed) const;
	//size_t psv(size_t p) const;
	//size_t nsv(size_t p) const;
	size_t length() const;

	void save(OutArchive& ar) const { bp.save(ar); }
	void load(InpArchive& ar) { bp.load(ar); }
	void clear() {bp.clear();}
};

template<typename RandomAccessIterator>
inline BitArray build_supercartisian_tree(bool minimum_tree, RandomAccessIterator first, RandomAccessIterator last) {
	BitArray bp = BitArrayBuilder::create(2 * std::distance(first, last));
	bp.fillzero();
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;
	std::stack<value_type> st;
	size_t p = 0;
	if (minimum_tree) {
		for (; first != last; ++first) {
			value_type val = *first;
			while ((!st.empty()) && (val < st.top())) { st.pop(); ++p; }
			st.push(val);
			bp.setbit(p, true);  p++;
		}
	} else {
		for (; first != last; ++first) {
			value_type val = *first;
			while ((!st.empty()) && (st.top() < val)) { st.pop(); ++p; }
			st.push(val);
			bp.setbit(p, true);  p++;
		}
	}
	return bp;
}

}//namespace
#endif //__RMQ_SUPER_CARTISIAN_TREE_H_