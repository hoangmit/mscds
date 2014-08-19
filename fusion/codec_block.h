#pragma once

/** 
Experimental implementation of fusion block
*/

#include "block_mem_mng.h"
#include "bitarray/bitstream.h"
#include "bitarray/bitop.h"

#include "codec/deltacoder.h"

#include "intarray/huffarray.h"
#include "generic_struct.h"

namespace mscds {


template<typename Model>
class CodeInterBlkBuilder: public InterBlockBuilderTp {
public:
	static const unsigned int SSBLKSIZE = 64;
	static const unsigned int ELEM_PER_BLK = 512;

	void start_model();
	void model_add(uint32_t val) { model.add(val); }

	void build_model();

	void init_bd(BlockBuilder& bd_);

	void register_struct();

	bool is_full() const { return cnt >= ELEM_PER_BLK; }
	bool is_empty() const { return cnt == 0; }

	void add(uint32_t val);

	void set_block_data(bool x = false);

	void debug_print_org(std::ostream& out = std::cout) const;

	void debug_print(std::ostream& out = std::cout) const;

	void build_struct();

	void deploy(StructIDList& lst);

private:
	unsigned int cnt;
	size_t total;
	std::vector<uint32_t> ptrs;
	OBitStream data_buffer, model_buffer;
	Model model;
	BlockBuilder * bd;
	unsigned int sid, did;
};

/// fused block for array of integers
template<typename Model>
class CodeInterBlkQuery: public InterBLockQueryTp {
public:
	static const unsigned int ELEM_PER_BLK = 512;
	CodeInterBlkQuery(): mng(nullptr) {}
	void clear() { mng = nullptr; sid = did = 0; len = 0; model.clear(); } 
	void setup(BlockMemManager& mng_, StructIDList& lst);

	uint64_t get(unsigned int i);

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		bool hasNext() const { return pos < data->len; }
		uint64_t next();
	private:
		unsigned int pos;
		IWBitStream is;
		const CodeInterBlkQuery * data;
		friend class CodeInterBlkQuery;

		void move_blk(unsigned int blk);
	};

	void getEnum(unsigned int pos, Enum * e) const {
		e->data = this;

		unsigned int blk = pos / ELEM_PER_BLK;
		unsigned int sbid = pos % ELEM_PER_BLK;
		unsigned int sblk = sbid / SSBLKSIZE;
		unsigned int px = sbid % SSBLKSIZE;
		e->pos = pos - px;

		auto br = mng->getData(did, blk);
		unsigned int w = br.bits(0, 8);
		unsigned int st = br.bits(8 + sblk * w, w);
		e->is.init(*br.ba, br.start + st);

		for (unsigned int i = 0; i < px; ++i)
			e->next();
	}

	size_t length() const { return len; }
	void inspect(const std::string &, std::ostream &) {}

	/*void debug_print(unsigned int blk, unsigned sscnt = 8, std::ostream& out = std::cout) const {
		auto br = mng->getData(did, blk);
		unsigned int w = get_w(blk);
		out << "Shortcut pointers: " << '\n';
		for (unsigned sblk = 0; sblk < sscnt; ++sblk) {
			unsigned int st = br.bits(sblk * w, w);
			out << st << ", ";
		}
		out << "\n\n";
	}*/
private:
	static const unsigned int SSBLKSIZE = 64;

	void load_model();
	Model model;
	uint64_t len;
	unsigned int sid, did;
	BlockMemManager* mng;
};

typedef CodeInterBlkBuilder<HuffmanModel> HuffBlkBuilder;

typedef CodeInterBlkQuery<HuffmanModel> HuffBlkQuery;

//----------------------------------------------------------------

template<typename Model>
void CodeInterBlkBuilder<Model>::start_model() { model.startBuild(); }

template<typename Model>
void CodeInterBlkBuilder<Model>::build_model() {
	model.endBuild();
	model.saveModel(&model_buffer);
}

template<typename Model>
void CodeInterBlkBuilder<Model>::init_bd(BlockBuilder &bd_) {
	bd = &bd_;
}

template<typename Model>
void CodeInterBlkBuilder<Model>::register_struct() {
	unsigned int bl = (model_buffer.length() + 7) / 8;
	bd->begin_scope("codec_array");
	sid = bd->register_summary(8 + bl, 0);
	did = bd->register_data_block();
	bd->end_scope();

	cnt = 0;
	total = 0;
}

template<typename Model>
void CodeInterBlkBuilder<Model>::add(uint32_t val) {
	assert(!is_full());
	if (cnt % SSBLKSIZE == 0)
		ptrs.push_back(data_buffer.length());
	model.encode(val, &data_buffer);
	cnt++;
	total++;
}

template<typename Model>
void CodeInterBlkBuilder<Model>::set_block_data(bool x) {
	if (is_empty()) return;
	//while (ptrs.size() < 9) ptrs.push_back(ptrs.back());
	unsigned char w = ceillog2(ptrs.back() + 1);
	unsigned int base = (ptrs.size())* w + 8;
	while (base + ptrs.back() >= (1u << w)) {
		w += 1;
		base = (ptrs.size())* w + 8;
	}
	data_buffer.close();
	bd->set_summary(sid);
	OBitStream& data = bd->start_data(did);
	data.puts(w, 8);
	for (unsigned int i = 0; i < ptrs.size(); ++i) {
		data.puts(ptrs[i] + base, w);
	}
	assert(ptrs.back() + base < (1u << w));
	data.append(data_buffer);
	//debug_print();
	ptrs.clear();
	data_buffer.clear();
	bd->end_data();
	cnt = 0;
}

template<typename Model>
void CodeInterBlkBuilder<Model>::debug_print_org(std::ostream &out) const {
	out << "Shortcut pointers: " << '\n';
	for (auto v : ptrs)
		out << v << ", ";
	out << '\n';
	unsigned w = ceillog2(ptrs.back() + 1);
	out << "W = " << w << '\n';
	unsigned int base = (ptrs.size())* w;
	out << "Base_Shift: " << base << "\n\n";
}

template<typename Model>
void CodeInterBlkBuilder<Model>::debug_print(std::ostream &out) const {
	out << "Shortcut pointers: " << '\n';
	for (auto v : ptrs)
		out << v << ", ";
	out << '\n';
	unsigned w = ceillog2(ptrs.back() + 1);
	out << "W = " << w << '\n';
	unsigned int base = (ptrs.size())* w;
	out << "Base_Shift: " << base << "\n\n";

	for (unsigned int i = 1; i < ptrs.size(); ++i) {
		auto d = ptrs[i] - ptrs[i-1] - 475;
		out << d << " ";
	}
	out << "\n\n";
}

template<typename Model>
void CodeInterBlkBuilder<Model>::build_struct() {
	if (!is_empty())
		set_block_data();
	model_buffer.puts(total);
	model_buffer.close();

	bd->set_global(sid, model_buffer);
	model_buffer.clear();
}

template<typename Model>
void CodeInterBlkBuilder<Model>::deploy(StructIDList &lst) {
	lst.addId("CodeInterBlk");
	lst.add(sid);
	lst.add(did);
}

template<typename Model>
void CodeInterBlkQuery<Model>::setup(BlockMemManager &mng_, StructIDList &lst) {
	mng = &mng_;
	lst.checkId("CodeInterBlk");
	sid = lst.get();
	did = lst.get();
	assert(sid > 0);
	assert(did > 0);
	load_model();
}

template<typename Model>
uint64_t CodeInterBlkQuery<Model>::get(unsigned int i) {
	Enum e;
	getEnum(i, &e);
	return e.next();
}

template<typename Model>
void CodeInterBlkQuery<Model>::load_model() {
	auto br = mng->getGlobal(sid);
	IWBitStream is;
	is.init(*br.ba, br.start);
	model.loadModel(is, true);
	len = is.get();
}

template<typename Model>
uint64_t CodeInterBlkQuery<Model>::Enum::next() {
	int64_t v = data->model.decode(&is);
	++pos;
	if (pos % ELEM_PER_BLK == 0) move_blk(pos / ELEM_PER_BLK);
	return v;
}

template<typename Model>
void CodeInterBlkQuery<Model>::Enum::move_blk(unsigned int blk) {
	if (!hasNext()) return;
	auto br = data->mng->getData(data->did, blk);
	unsigned int w = br.bits(0, 8);
	unsigned int st = br.bits(8, w);
	is.init(*br.ba, br.start + st);
}

}//namespace
