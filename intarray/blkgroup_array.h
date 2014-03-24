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
};


struct BitRange {
	BitRange() : ba(nullptr), start(0), len(0) {}

	BitRange(BitRange& other) : ba(other.ba), start(other.start), len(other.len) {}
	BitRange(BitArray* ba_, size_t start_, size_t len_) : ba(ba_), start(start_), len(len_) {}

	uint64_t bits(size_t start_, size_t len_) const {
		assert(start_ <= start && start_ + len_ <= start + len);
		return ba->bits(start + start_, len_);
	}

	void setbits(size_t start_, uint64_t value, unsigned int len_) {
		assert(start_ <= start && start_ + len_ <= start + len);
		ba->setbits(start + start_, value, len_);
	}

	BitArray* ba;
	size_t start, len;
};

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
		IWBitStream inp;
		inp.init(ba, pt);
		w = inp.get(8);
		start_.resize(1);
		start_[0] = 0;
		for (unsigned i = 0; i < count_; ++i)
			start_.push_back(start_.back() + inp.get(w));
		valid = true;
	}

	unsigned int ptr_space() const {
		return 8 + count_ * w;
	}

	//------------------------------------------------------------------------

	unsigned int start(unsigned int i) const { return start_[i]; }
	unsigned int end(unsigned int i) const { return start_[i+1]; }
	unsigned int length(unsigned int i) const { return start_[i+1] - start_[i]; }
	

private:
	bool valid;
	unsigned int count_, w;
	std::vector<unsigned int> start_;
};

class BlockBuilder;


class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int id) {
		return BitRange();
	}

	BitRange getSummary(size_t blk, unsigned int id) {
		assert(blk < blkcnt);
		size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
		stp *= 8;
		return BitRange(&summary, stp + summary_ps[id] * 8, (summary_ps[id + 1] - summary_ps[id]) *8);
	}

	BitRange getData(size_t blk, unsigned int id) {
		assert(blk < blkcnt);
		size_t stp = header_size + global_struct_size + blk * summary_chunk_size;
		stp *= 8;
		stp += summary_chunk_size * 8 - sizeof(uint64_t) * 8;
		uint64_t ptrx = summary.bits(stp, 64);
		bptr.loadBlock(data, ptrx, 0);
		size_t base = ptrx + bptr.ptr_space();
		return BitRange(&data, base + bptr.start(id), bptr.length(id));
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
	std::vector<std::string> info;
	size_t summary_chunk_size, global_struct_size, header_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};

class BlockBuilder {
public:
	unsigned int register_struct(size_t global_size, size_t summary_blk_size, const std::string& str_info = "");

	//-----------------------------------------------------
	OBitStream& globalStructData();

	void init_data();

	OBitStream& start_struct(unsigned int id, const MemRange& r);
	void finish_struct();
	void finish_block();
	void build(BlockMemManager* mng);
	BlockBuilder();
private:
	std::vector<std::string> info;
	std::vector<unsigned int> summary_sizes, global_sizes;

	FixBlockPtr bptr;
	OBitStream header, summary, data, buffer;

	size_t blkcnt;
	size_t cid, last_pos;
	uint64_t start_ptr;
	
	size_t summary_chunk_size, global_struct_size, header_size;
	bool finish_reg;
};

}//namespace
