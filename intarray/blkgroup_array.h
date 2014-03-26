#pragma once


#include "codec/stream/codec_adapter.h"
#include "framework/archive.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace mscds {

class BlockAPI {
public:
	void buildBlock();

	void saveBlock(OBitStream * bs);
	void loadBlock(BitArray ba, size_t pt, size_t);

};

struct MemRange {
	char* ptr;
	size_t len;
	MemRange() : ptr(nullptr), len(0) {}
	MemRange(char* ptr_, size_t len_) : ptr(ptr_), len(len_) {}
	MemRange(const MemRange& m) : ptr(m.ptr), len(m.len) {}

	template<typename T>
	static MemRange wrap(T& t) { return MemRange((char*) &t, sizeof(T)); }
	template<typename T>
	static MemRange wrap2(T& t, size_t len) {
		assert(len <= sizeof(T) * CHAR_BIT);
		MemRange((char*)&t, sizeof(T));
	}
};


struct BitRange {
	BitRange() : ba(nullptr), start(0), len(0) {}
	BitRange(const BitRange& other) : ba(other.ba), start(other.start), len(other.len) {}
	BitRange(BitArray* ba_, size_t start_, size_t len_) : ba(ba_), start(start_), len(len_) {}

	uint64_t bits(size_t start_, size_t len_) const {
		assert(start_ <= start && start_ + len_ <= start + len);
		return ba->bits(start + start_, len_);
	}

	void setbits(size_t start_, uint64_t value, unsigned int len_) {
		assert(start_ <= start && start_ + len_ <= start + len);
		ba->setbits(start + start_, value, len_);
	}

	BitRange inc_front(unsigned int l) const {
		assert(l <= len);
		return BitRange(ba, start + l, len - l);
	}

	BitArray* ba;
	size_t start, len;
};

//----------------------------------------------------------------------

struct VBytePtr {
	std::vector<unsigned int> vals;
	void add(unsigned int v) { vals.push_back(v); }
	void _build();
	void _init();
	void saveArray(OBitStream* bs) {
		VByteArray::append(*bs, vals.size());
	}
	void saveData(OBitStream * bs) {
		for (unsigned int i = 0; i < vals.size(); ++i)
			VByteArray::append(*bs, vals[i]);
	}

	void loadArray(BitRange& br) {}
	void loadData(unsigned int len, BitRange& br) {}

	unsigned int get(unsigned int id) { return vals[id]; }
	void clear() { vals.clear(); }
};

struct FixWidthPtr {

};

struct FixSumPtr {

};
//----------------------------------------------------------------------

class FixBlockPtr {
public:
	FixBlockPtr() {}

	void init(unsigned int nb) { start_.resize(nb + 1); count_ = nb, valid = false; }

	void set(unsigned int i, unsigned int blksize) { start_[i + 1] = blksize; }

	void _build() {
		valid = true;
		start_[0] = 0;
		for (unsigned int i = 1; i <= count_; ++i) {
			start_[i] = start_[i - 1] + start_[i];
		}
	}
	void saveBlock(OBitStream * bs) {
		if (!valid) _build();

		assert(start_.size() == count_ + 1);
		unsigned int w = 0;
		for (unsigned i = 0; i < count_; ++i)
			w = std::max<unsigned>(ceillog2(1 + start_[i+1] - start_[i]), w);
		bs->puts(w, 8);
		for (unsigned i = 0; i < count_; ++i)
			bs->puts(start_[i+1] - start_[i], w);
	}

	void reset() {
		valid = false;
	}

	void loadBlock(BitArray& ba, size_t pt, size_t) {
		w = ba.bits(pt, 8);
		start_.resize(1);
		start_[0] = 0;
		for (unsigned i = 0; i < count_; ++i)
			start_.push_back(start_.back() + ba.bits(pt + 8 + i * w, w)); 
		valid = true;
	}

	unsigned int ptr_space() const {
		return 8 + count_ * w;
	}

	//------------------------------------------------------------------------

	unsigned int start(unsigned int i) const { return start_[i]; }
	unsigned int end(unsigned int i) const { return start_[i+1]; }
	unsigned int length(unsigned int i) const { return start_[i+1] - start_[i]; }
	
	void clear() { valid = false; start_.clear(); }

private:
	bool valid;
	unsigned int count_, w;
	std::vector<unsigned int> start_;
};

class BlockBuilder;


class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int gid) {
		return BitRange(&summary, header_size * 8, global_struct_size * 8);
	}

	BitRange getSummary(size_t blk, unsigned int sid) {
		assert(blk < blkcnt);
		size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
		stp *= 8;
		return BitRange(&summary, stp + summary_ps[sid] * 8, (summary_ps[sid + 1] - summary_ps[sid]) * 8);
	}

	BitRange getData(size_t blk, unsigned int did) {
		assert(blk < blkcnt);
		if (last_blk != blk) {
			size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
			stp += summary_chunk_size - sizeof(uint64_t);
			stp *= 8;
			uint64_t ptrx = summary.bits(stp, 64);
			bptr.loadBlock(data, ptrx, 0);
			last_blk = blk;
			last_ptrx = ptrx;
		}
		size_t base = last_ptrx + bptr.ptr_space();
		return BitRange(&data, base + bptr.start(did), bptr.length(did));
	}

	void save(mscds::OutArchive& ar) const {
		ar.startclass("fusion_block_manager", 1);
		ar.var("struct_count").save(str_cnt);
		ar.var("block_count").save(blkcnt);
		summary.save(ar.var("summary_data"));
		std::string s = std::accumulate(info.begin(), info.end(), std::string());
		data.save(ar.var("data" + s));
		ar.endclass();
	}

	void load(mscds::InpArchive& ar) {
		ar.loadclass("fusion_block_manager");
		ar.var("struct_count").load(str_cnt);
		ar.var("block_count").load(blkcnt);
		summary.load(ar.var("summary_data"));
		//std::string s = std::accumulate(info.begin(), info.end(), std::string());
		data.load(ar);
		ar.endclass();
	}
	void clear() {
		bptr.clear();
		global_ps.clear(); summary_ps.clear();
		summary.clear(); data.clear();

		blkcnt = 0; str_cnt = 0;
	}

private:
	static std::vector<unsigned int> prefixsum_vec(const std::vector<unsigned int>& v) {
		std::vector<unsigned int> out(v.size() + 1);
		out[0] = 0;
		for (size_t i = 1; i <= v.size(); ++i)
			out[i] = out[i - 1] + v[i - 1];
		return out;
	}
	void init();
	
private://essensial data
	BitArray summary;
	BitArray data;
	
	size_t blkcnt;
	size_t str_cnt;
private:
	size_t last_blk;
	uint64_t last_ptrx;
	std::vector<std::string> info;
	size_t summary_chunk_size, global_struct_size, header_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};

class BlockBuilder {
public:
	unsigned int register_data_block();
	unsigned int register_summary(size_t global_size, size_t summary_blk_size, const std::string& str_info = "");

	//-----------------------------------------------------
	void set_global(unsigned int sid, const MemRange& r);

	void init_data();

	void start_block();

	void set_summary(unsigned int sid, const MemRange& r);

	OBitStream& start_data(unsigned int did);
	void end_data();

	void end_block();

	void build(BlockMemManager* mng);

	BlockBuilder();
	void clear();
private:
	std::vector<std::string> info;
	std::vector<unsigned int> summary_sizes, global_sizes;

	FixBlockPtr bptr;
	OBitStream header, summary, data, buffer;

	size_t blkcnt;
	size_t scid, bcid, gcid, last_pos;
	uint64_t start_ptr;

	size_t n_data_block;
	
	size_t summary_chunk_size, global_struct_size, header_size;
	bool finish_reg;
};

}//namespace
