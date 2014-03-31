#pragma once

#include "blkgroup_array.h"
#include "bitarray/bitstream.h"
#include "bitarray/bitop.h"

namespace mscds {

template<typename Model>
class CodeInterBlkBuilder {
public:
	void start_model() { model.startBuild(); }
	void model_add(uint32_t val) { model.add(val); }
	void build_model() {
		model.endBuild();
		model.saveModel(&buffer);
	}

	void register_struct() {
		unsigned int bl = (buffer.length() + 7 ) / 8;
		sid = bd.register_summary(bl, 1);
		did = bd.register_data_block();
		buffer.clear();
		ptrs.push_back(buffer.length());
		assert(0==ptrs[0]);
		cnt = 0;
	}

	void add(uint32_t val) {
		model.encode(val, buffer);
		cnt++;
		if (cnt % 64 == 0)
			ptrs.push_back(buffer.length());
	}

	void set_block_data() {
		if (cnt == 0) return ;
		cnt = 0;
		//while (ptrs.size() < 9) ptrs.push_back(ptrs.back());
		unsigned char w = ceillog(ptrs.back() + 1);
		bd.set_summary(sid, MemRange::wrap(w));
		OBitStream& data; bd.start_data(did);
		assert(ptrs.back() <= (1ull << w));
		unsigned int base = (ptrs.size() - 1)* w;
		for (unsigned int i = 1; i < ptrs.size(); ++i) {
			data.put(ptrs[i] + base, w);
		}
		data.append(buffer);
		bd.end_data();
	}

	void build_struct() {
		set_block_data();
	}

	void deploy(StructIDList& lst) {

	}

private:
	unsigned int cnt;
	std::vector<uint32_t> ptrs;
	OBitStream buffer;
	Model model;
	BlockBuilder bd;
	unsigned int sid, did;
};

}//namespace
