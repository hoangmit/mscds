#pragma once

#include "intarray/blkgroup_array.h"
#include "intarray/fuse_blk_model.h"
#include "cwig/intv/nintv_fuse.h"

namespace app_ds {
struct XBuilder {
	typedef mscds::LiftStBuilder<NIntvInterBlkBuilder,
			mscds::CodeInterBlkBuilder
		> A;
	A x;
	void test() {
		
	}
};

}//namespace