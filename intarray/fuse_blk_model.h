#pragma once

#include "blkgroup_array.h"
#include "bitarray/bitstream.h"
#include "bitarray/bitop.h"

#include "huffarray.h"

namespace mscds {

typedef HuffmanModel Model;

//template<typename Model>
class CodeInterBlkBuilder {
public:
	void start_model() { model.startBuild(); }
	void model_add(uint32_t val) { model.add(val); }
	void build_model() {
		model.endBuild();
		model.saveModel(&model_buffer);
	}

	void register_struct() {
		unsigned int bl = (model_buffer.length() + 7) / 8;
		sid = bd.register_summary(8 + bl, 1);
		did = bd.register_data_block();

		cnt = 0;
	}

	void add(uint32_t val) {
		model.encode(val, &data_buffer);
		cnt++;
		if (cnt % 64 == 0)
			ptrs.push_back(data_buffer.length());
	}

	void set_block_data() {
		if (cnt == 0) return ;
		cnt = 0;
		//while (ptrs.size() < 9) ptrs.push_back(ptrs.back());
		unsigned char w = ceillog2(ptrs.back() + 1);
		bd.set_summary(sid, MemRange::wrap(w));
		OBitStream& data = bd.start_data(did);
		assert(ptrs.back() <= (1ull << w));
		unsigned int base = (ptrs.size())* w;
		for (unsigned int i = 0; i < ptrs.size(); ++i) {
			data.puts(ptrs[i] + base, w);
		}
		data.append(data_buffer);
		bd.end_data();
	}

	void build_struct() {
		set_block_data();
		model_buffer.put(cnt);
		model_buffer.close();

		bd.set_global(sid, model_buffer);
		model_buffer.clear();
	}

	void deploy(StructIDList& lst) {
		lst.addId("CodeInterBlkBuilder");
		lst.add(sid);
		lst.add(did);
	}

private:
	unsigned int cnt;
	std::vector<uint32_t> ptrs;
	OBitStream data_buffer, model_buffer;
	Model model;
	BlockBuilder bd;
	unsigned int sid, did;
};



//template<typename Model>
class CodeInterBlkQuery {
public:
	CodeInterBlkQuery(): mng(nullptr) {}
	void setup(BlockMemManager& mng_, StructIDList& lst) {
		mng = &mng_;
		lst.checkId("CodeInterBlkBuilder");
		sid = lst.get();
		did = lst.get();
		assert(sid > 0);
		assert(did > 0);
		load_model();
	}

	uint64_t get(unsigned int i) {
		Enum e;
		getEnum(i, &e);
		return e.next();
	}

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		bool hasNext() const { return true; }
		uint64_t next() {
			auto v = data->model.decode(&is);
			++pos;
			if (pos % 512 == 0) move_blk(0);
			return v;
		}
	private:
		unsigned int pos;
		IWBitStream is;
		const CodeInterBlkQuery * data;
		friend class CodeInterBlkQuery;

		void move_blk(unsigned int blk) {
			auto br = data->mng->getData(data->did, blk);
			unsigned int w = data->get_w(blk);
			unsigned int st = br.bits(0, w);
			is.init(*br.ba, br.start + st);
		}
	};

	void getEnum(unsigned int pos, Enum * e) const {
		e->data = this;
		
		unsigned int blk = pos / 512;
		unsigned sbid = pos % 512;
		unsigned int sblk = sbid / SSBLKSIZE;
		unsigned int px = sbid % SSBLKSIZE;
		e->pos = pos - px;

		auto br = mng->getData(did, blk);
		unsigned int w = get_w(blk);
		unsigned int st = br.bits(sblk * w, w);
		e->is.init(*br.ba, br.start + st);

		for (unsigned int i = 1; i < px; ++i) e->next();
	}
	void inspect(const std::string& cmd, std::ostream& out) const {}
private:
	static const unsigned int SSBLKSIZE = 64;

	unsigned char get_w(unsigned int blk) const {
		return mng->getSummary(sid, blk).byte(0);
	}

	void load_model() {
		auto br = mng->getGlobal(sid);
		IWBitStream is;
		is.init(*br.ba, br.start);
		model.loadModel(is, true);
		len = is.get();
	}
	Model model;
	uint64_t len;
	unsigned int sid, did;
	BlockMemManager* mng;
};




}//namespace
