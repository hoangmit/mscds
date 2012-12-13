#pragma once

#ifndef __RMQ_SUPER_CARTISIAN_TREE_H_
#define __RMQ_SUPER_CARTISIAN_TREE_H_

#include "BP_bits.h"
#include <cassert>

namespace mscds {

BitArray build_supercartisian_tree(const std::vector<uint64_t>& arr, bool mintree);

class RMQ_sct {

	BP_aux bp;
public:
	void build(const std::vector<uint64_t>& arr) {
		BitArray b = build_supercartisian_tree(arr, true);
		bp.build(b, 128);
	}

	size_t m_idx(size_t st, size_t ed) const {
		ed -= 1;
		assert(st <= ed && ed < bp.length());
		if (st == ed) return st;
		size_t i = bp.select(st);
		size_t j = bp.select(ed);
		size_t fc_i = bp.find_close(i);
		if (j < fc_i) {
			return st;
		} else {
			size_t ec = bp.rr_enclose(i,j);
			if (ec == BP_block::NOTFOUND)
				return ed;
			else 
				return bp.rank(ec);
		}
	}
	OArchive& save(OArchive& ar) const;
	IArchive& load(IArchive& ar);
};


}//namespace
#endif //__RMQ_SUPER_CARTISIAN_TREE_H_