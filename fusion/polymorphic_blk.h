
#pragma once

/**
Experimental implementation of fusion block
*/

#include <cassert>
#include "fusion/blkgroup_array.h"

namespace mscds {

//only zerosize header interblocks can be combined
template<typename B1, typename B2>
class PolyIBlockBuilder {
	BlockBuilder* bd;
	std::pair<unsigned int, unsigned int> rp1, rp2, rp3;
public:
	B1 b1;
	B2 b2;
	unsigned int choice;
	void choose(unsigned int opt) {
		assert(opt == 1 || opt == 2);
		choice = opt;
	}

	void register_struct() {
		rp1 = bd->current_reg_numbers();
		b1.register_struct();
		rp2 = bd->current_reg_numbers();
		b2.register_struct();
		rp3 = bd->current_reg_numbers();
	}

	void set_block_data() {
		if (choice == 1) {
			b1.set_block_data();
			//skip the other
			for (unsigned int i = rp2.first + 1; i <= rp3.second; ++i) 
				bd->set_summary(i);
			for (unsigned int i = rp2.second + 1; i <= rp3.second; ++i) {
				bd->start_data(i);
				bd->end_data();
			}
		} else if (choice == 2) {
			b2.set_block_data();
			//skip the other
			for (unsigned int i = rp1.first + 1; i <= rp2.second; ++i)
				bd->set_summary(i);
			for (unsigned int i = rp1.second + 1; i <= rp2.second; ++i) {
				bd->start_data(i);
				bd->end_data();
			}
		} else {
			assert(false);
		}
	}

	void build_struct() {
		if (choice == 1) {
			b1.build_struct();
			for (unsigned int i = rp2.first + 1; i <= rp3.second; ++i)
				bd->set_global(i);
		} else
		if (choice == 2) {
			b2.build_struct();
			for (unsigned int i = rp1.first + 1; i <= rp2.first; ++i)
				bd->set_global(i);
		}
		else {
			assert(false);
		}
	}

	void deploy(StructIDList& lst) {
		lst.addId("poly");
		b1.deploy(lst);
		b2.deploy(lst);
	}

};

template<typename B1, typename B2>
class PolyIBlock {
public:
};


}//namespace

