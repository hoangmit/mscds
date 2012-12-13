#include "RMQ_sct.h"

#include <stack>
#include <vector>

namespace mscds {
BitArray build_supercartisian_tree(const std::vector<uint64_t>& arr, bool mintree){
	BitArray bp = BitArray(arr.size() * 2);
	bp.fillzero();
	std::stack<uint64_t> st;
	size_t p = 0;
	for (size_t i = 0; i < arr.size(); ++i) {
		uint64_t l = arr[i];
		if (mintree) {
			while (st.size() > 0 && l < st.top()) {
				st.pop();
				++p;
			}
		} else {
			while ((!st.empty()) && l > st.top()) {
				st.pop();
				++p;
			}
		}
		st.push(l);
		bp.setbit(p, true);
		p++;
	}
	while (!st.empty()) {
		st.pop();
		bp.setbit(p, false);
		p++;
	}
	assert(p == 2*arr.size());
	return bp;
}



}//namespace