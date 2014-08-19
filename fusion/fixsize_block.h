#pragma once

/** 
Experimental implementation of fusion block
*/

#include "generic_struct.h"
namespace mscds {

template<typename T>
class SmallFixsizeBlockBuilder: public InterBlockBuilderTp {
	BlockBuilder * bd;
	unsigned int sid, did;

	size_t headersz, blksz;
	std::vector<T> lst;
	unsigned int len, i;
public:
	void init(size_t n) {
		len = n;
	}

	void add(T& v) {
		lst[i] = v;
		++i;
		assert(i <= len);
	}

	void init_bd(BlockBuilder& bd_) { bd = &bd_; }

	bool is_empty() const { return i == 0; }

	bool is_full() const { return i == len; }

	void register_struct() {
		sid = bd->register_summary(0, 0);
		did = bd->register_data_block();
	}

	void set_block_data(bool lastblock = false) {
		bd->set_summary(sid);
		OBitStream& out = bd->start_data(did);
		out.puts_c((const char*) lst.data(), sizeof(T) * len);
		bd->end_data();
		i = 0;
	}

	void build_struct() {
		bd->set_global(sid);
	}

	void deploy(StructIDList& lst) {
		lst.addId("fixsize");
		lst.add(sid);
		lst.add(did);
	}
};


template<typename T>
class SmallFixsizeBlock:  public InterBLockQueryTp {
	void clear() {}

	void setup(BlockMemManager& mng_, StructIDList& lst) {

	}

	T get(unsigned int i) {
	}
};

}//namespace
