#include "RMQ_sct.h"

#include <stack>
#include <vector>

namespace mscds {
	size_t RMQ_sct::m_idx(size_t st, size_t ed) const {
		ed--;
		assert(st <= ed && ed < length());
		if (st == ed) return st;
		size_t i = bp.select(st);
		size_t j = bp.select(ed);
		size_t fstclose = bp.find_close(i);
		if (j < fstclose) {
			return st;
		} else {
			size_t p = bp.rr_enclose(i, j);
			if (p != BP_block::NOTFOUND)
				return bp.rank(p);
			else
				return ed;		
		}
	}

	size_t RMQ_sct::length() const {
		return bp.length() / 2;
	}


}//namespace