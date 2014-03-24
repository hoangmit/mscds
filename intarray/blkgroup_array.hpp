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

	}
	void load(mscds::InpArchive& ar) {

	}
private:
	static std::vector<unsigned int> prefixsum_vec(const std::vector<unsigned int>& v) {
		std::vector<unsigned int> out(v.size() + 1);
		out[0] = 0;
		for (size_t i = 1; i <= v.size(); ++i)
			out[i] = out[i - 1] + v[i - 1];
		return out;
	}
	void init() {
		std::vector<unsigned int> summary_sizes, global_sizes;
		IWBitStream is(summary);
		//VByteArray::load(is, summary_sizes);
		unsigned int n = 0;
		n = VByteArray::extract(is);
		assert(n < 128);
		for (unsigned int i = 0; i < n; ++i)
			global_sizes.push_back(VByteArray::extract(is));
		for (unsigned int i = 0; i < n; ++i)
			summary_sizes.push_back(VByteArray::extract(is));
		header_size = is.get(16);
		header_size += 2;
		assert((header_size * 8) == is.extracted());
		bptr.init(n);
		summary_ps = prefixsum_vec(summary_sizes);
		global_ps = prefixsum_vec(global_sizes);
		global_struct_size = global_ps.back();
		summary_chunk_size = summary_ps.back() + sizeof(uint64_t);
	}
private://essensial data
	BitArray summary;
	BitArray data;
	
	size_t blkcnt;
private:
	size_t summary_chunk_size, global_struct_size, header_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};

class BlockBuilder {
public:
	unsigned int register_struct(size_t global_size, size_t summary_blk_size) {
		summary_sizes.push_back(summary_blk_size);
		global_sizes.push_back(global_size);
		summary_chunk_size += summary_blk_size;
		return summary_sizes.size() - 1;
	}

	//-----------------------------------------------------
	OBitStream& globalStructData() {
		return header;
	}

	void init_data() {
		//initialize variables
		finish_reg = true; start_ptr = 0;
		assert(summary_sizes.size() < 128);
		bptr.init(summary_sizes.size());
		//save array
		assert(global_sizes.size() == summary_sizes.size());
		VByteArray::append(header, global_sizes.size());
		for (unsigned int i = 0; i < global_sizes.size(); ++i)
			VByteArray::append(header, global_sizes[i]);
		for (unsigned int i = 0; i < summary_sizes.size(); ++i)
			VByteArray::append(header, summary_sizes[i]);
		//save global here
		header_size = header.length() / 8;
		assert(header_size < (1<<16));
		header.puts(header_size, 16);
		//compute summary chunk size
		summary_chunk_size += sizeof(uint64_t);
		header_size = header.length() / 8;
		global_struct_size = std::accumulate(global_sizes.begin(), global_sizes.end(), 0);
		cid = 0;
	}

	OBitStream& start_struct(unsigned int id, const MemRange& r) {
		assert(cid == id);
		assert(r.len <= summary_sizes[cid]);
		summary.puts_c(r.ptr, r.len);
		if (r.len < summary_sizes[cid])
			summary.put0(summary_sizes[cid] - r.len);
		last_pos = buffer.length();
		return buffer;
	}

	void finish_struct() {
		bptr.set(cid, buffer.length() - last_pos);
		last_pos = buffer.length();
		cid++;
	}

	void finish_block() {
		if (cid != summary_sizes.size()) throw std::runtime_error("mis");
		buffer.close();
		bptr.saveBlock(&data);
		data.append(buffer);
		buffer.clear();

		summary.puts_c((const char*)&start_ptr, sizeof(start_ptr));
		start_ptr = data.length();

		cid = 0;
		blkcnt++;
		bptr.reset();
	}

	void build(BlockMemManager* mng) {
		if (header.length() != (header_size + global_struct_size) * 8)
			throw std::runtime_error("size mismatch");
		header.append(summary);
		header.build(&mng->summary);
		data.build(&mng->data);
		mng->blkcnt = blkcnt;
		mng->init();
	}

	BlockBuilder() : blkcnt(0), finish_reg(false) {
		summary_chunk_size = 0;
	}

private:
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
