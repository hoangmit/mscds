
#pragma once

#include "blkgroup_array.h"

namespace mscds {

class PtrInterBlk {
public:
	void init_bd(BlockBuilder& bd) {}
	void register_struct() {}
	void set_block_data() {}
	void build_struct() {}
	void deploy(StructIDList & lst) {}
};

}//namespace