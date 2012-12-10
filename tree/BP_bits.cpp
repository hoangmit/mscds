#include "BP_bits.h"

#include <stack>
#include <utility>


namespace mscds {
BitArray pioneer_map(const BitArray& bp, size_t blksize) {
	std::stack<size_t> opens;
	std::pair<size_t, size_t> lastfar;
	bool haslast = false;
	BitArray pimap = BitArray::create(bp.length());
	pimap.fillzero();
	size_t pioneer_cnt = 0;
	for (size_t i = 0; i < bp.length(); ++i) {
		if (bp.bit(i)) {
			opens.push(i);
		} else {
			size_t op = opens.top();
			opens.pop();
			if (op/blksize != i/blksize) { // far
				if (haslast && lastfar.first/blksize != op/blksize) {
					// pioneer
					pimap.setbit(lastfar.first, true);
					pimap.setbit(lastfar.second, true);
					pioneer_cnt++;
				}
				haslast = true;
				lastfar = std::make_pair(op, i);
			}
		}
		if ((i + 1)/blksize != i/blksize) {
			if (haslast) {
				// pioneer
				pimap.setbit(lastfar.first, true);
				pimap.setbit(lastfar.second, true);
				pioneer_cnt++;
			}
			haslast = false;
		}
	}
	if (haslast) {
		// pioneer
		pimap.setbit(lastfar.first, true);
		pimap.setbit(lastfar.second, true);
		pioneer_cnt++;
	}
	assert(pioneer_cnt <= 4 * bp.length()/blksize - 6);
	return pimap;
}

}//namespace