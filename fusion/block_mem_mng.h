#pragma once

/** 
Experimental implementation of fusion structure

*/

#include "codec/stream/codec_adapter.h"
#include "framework/archive.h"
#include "inc_ptrs.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <queue>
#include <tuple>
#include <climits>

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
		assert(start + start_ + len_ <= start + len);
		return ba->bits(start + start_, len_);
	}

	uint8_t byte(unsigned int i = 0) const {
		return bits(8*i, 8);
	}

	uint64_t word(unsigned int i = 0) const {
		return bits(64*i, 64);
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


class BlockMemManager;

class BlockBuilder {
public:
	// register number start from 1
	unsigned int register_data_block();
	unsigned int register_summary(size_t global_size, size_t summary_blk_size, const std::string& str_info = "");
	std::pair<unsigned int, unsigned int> current_reg_numbers();

	//-----------------------------------------------------

	void init_data();

	void set_global(unsigned int sid);
	void set_global(unsigned int sid, const MemRange& r);
	void set_global(unsigned int sid, const OBitStream& r);

	void start_block();

	void set_summary(unsigned int sid);
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

class StructIDList {
public:
	StructIDList(): pfront(0) {}
	StructIDList(const StructIDList& other): pfront(0), _lst(other._lst) {}

	void addId(const std::string& name);
	void checkId(const std::string& name);
	void add(unsigned int id);
	unsigned int get();

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();
	void reset();

	unsigned int pfront;
	std::deque<int> _lst;
};


class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int gid) {
		assert(gid > 0 && gid <= global_ps.size());
		return BitRange(&summary, header_size + global_ps[gid - 1], (global_ps[gid] - global_ps[gid - 1]));
	}

	BitRange getSummary(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_ps.size());
		assert(blk < blkcnt);
		size_t stp = prefix_size + blk * summary_chunk_size;
		return BitRange(&summary, stp + summary_ps[sid - 1], (summary_ps[sid] - summary_ps[sid - 1]));
	}

	uint64_t summary_word(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_ps.size());
		assert(blk < blkcnt);
		size_t stp = prefix_size + blk * summary_chunk_size;
		return summary.bits64(stp + summary_ps[sid - 1]);
	}

	BitRange getData(unsigned int did, size_t blk) {
		assert(did > 0 && did <= str_cnt);
		assert(blk < blkcnt);
		did -= 1;
		if (last_blk != blk) {
			size_t stp = prefix_size + blk * summary_chunk_size;
			stp += summary_chunk_size - sizeof(uint64_t) * 8;
			uint64_t ptrx = summary.bits(stp, 64);
			bptr.loadBlock(data, ptrx, 0);
			last_blk = blk;
			last_ptrx = ptrx;
		}
		size_t base = last_ptrx + bptr.ptr_space();
		return BitRange(&data, base + bptr.start(did), bptr.length(did));
	}

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();
	void inspect(const std::string &cmd, std::ostream &out);

private:
	static std::vector<unsigned int> prefixsum_vec(const std::vector<unsigned int>& v);
	void init();
	
private:   //essensial data
	BitArray summary;
	BitArray data;
	
	size_t blkcnt;
	size_t str_cnt;
private:
	size_t last_blk;
	uint64_t last_ptrx;
	std::vector<std::string> info;
	size_t summary_chunk_size, global_struct_size, header_size, prefix_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};

}//namespace
