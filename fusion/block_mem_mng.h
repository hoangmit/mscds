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
class FixedSizeMemBuilder {
public:
	unsigned int declare_blk(size_t size) {
		_sizes.push_back(size);
	}

	void init_data() {
		assert(_size.size() < 128);
		gcid = 0;
		out.clear();
	}

	void add_block_data(unsigned int id) {
		assert(id == gcid + 1);
		if (_sizes[id] > 0)
			out.put0(8*_sizes[id]);
		gcid++;
	}

	void add_block_data(unsigned int sid, const MemRange& r) {
		assert(r.len <= global_sizes[gcid]);
		out.puts_c(r.ptr, r.len);
		if (r.len < _sizes[gcid])
			out.put0(8*(_sizes[gcid] - r.len));
		gcid++;
	}

	void add_block_data(unsigned int sid, const OBitStream& os) {
		assert(sid == gcid + 1);
		assert(os.length() <= global_sizes[gcid] * 8);
		out.append(os);
		if (os.length() < _sizes[gcid] * 8)
			out.put0(8*(_sizes[gcid] * 8 - os.length()));
		gcid++;
	}

	void store_sizes(mscds::OBitStream& buf) {
		assert(_size.size() < 128);
		VByteStream::append(buf, _sizes.size());
		for (unsigned int i = 0; i < _sizes.size(); ++i)
			VByteStream::append(buf, _sizes[i]);
	}

	void store_data(mscds::OBitStream& dout) {
		dout.append(out);
		out.clear();
	}

	void clear() { _sizes.clear(); out.clear(); gcid = 0; }
private:
	std::vector<unsigned int> _sizes;
	OBitStream out;
	unsigned int gcid;
};

class FixedSizeMemQuery {
public:
	size_t load_sizes(BitArray& b, size_t pos = 0) {
		size_t ipos = pos;
		size_t n = VByteStream::extract(b, ipos);
		for (unsigned int i = 0; i < n; ++i)
			_sizes.push_back(VByteStream::extract(b, ipos));
		ps_sz.resize(_sizes.size() + 1);
		ps_sz[0] = 0;
		for (size_t i = 1; i <= _sizes.size(); ++i)
			ps_sz[i] = ps_sz[i - 1] + _sizes[i - 1];
		_chunk_size = _sizes.back();
		return ipos - pos;
	}

	size_t bind_data(BitArray& b, size_t pos = 0) {}

	size_t getDataLoc(unsigned int id, unsigned int index) {
		size_t stp = index * _chunk_size;
		return stp + ps_sz[id - 1];
	}
	std::pair<unsigned int, unsigned int> getDataRange(unsigned int id, unsigned int index) {
		size_t stp = index * _chunk_size;
		return std::pair<unsigned int, unsigned int>(stp + ps_sz[id - 1], stp + ps_sz[id]);
	}
private:
	std::vector<unsigned int> _sizes, ps_sz;
	unsigned _chunk_size;
};

//----------------------------------------------------------------------

class BlockMemManager;

#define __REPORT_FUSION_BLOCK_SIZE__


struct BlockInfoReport {
	BlockInfoReport(): header_overhead(0), block_overhead(0), blkcnt(0) {}
	enum InfoItemType { NOTHING, OPEN_SCOPE, CLOSE_SCOPE, GLOBAL, HEADER, DATA };
	struct InfoItem {
		InfoItem(): type(NOTHING), size(0) {};
		InfoItem(InfoItemType _type): type(_type), size(0) {}
		InfoItem(InfoItemType _type, const std::string& _name): type(_type), name(_name), size(0) {}
		InfoItem(InfoItemType _type, const std::string& _name, size_t sz): type(_type), name(_name), size(sz) {}

		InfoItem(const InfoItem& other) {
			this->name = other.name;
			this->type = other.type;
			this->size = other.size;
		}

		std::string name;
		InfoItemType type;
		size_t size;

	};
	std::vector<InfoItem> infolst;
	std::vector<size_t> datainfoloc;

	size_t header_overhead;
	size_t block_overhead;

	size_t blkcnt;

	void set_data_size(unsigned int did, size_t sz) {
		#ifdef __REPORT_FUSION_BLOCK_SIZE__
			infolst[datainfoloc[did - 1]].size += sz;
		#endif
	}

	void clear() {
		infolst.clear();
		datainfoloc.clear();
	}

	void report(std::ostream& out) {
		out << "<block_structure count=\"" << blkcnt << "\">";
		out << "<header name = \"__pointer_overhead__\" bitsize=\"" << header_overhead * blkcnt << "\" />";
		out << "<data name = \"__pointer_overhead__\" bitsize=\"" << block_overhead << "\" />";
		for (unsigned int i = 0; i < infolst.size(); ++i) {
			const auto & x = infolst[i];
			if (x.type == OPEN_SCOPE)
				out << "<scope name=\"" << x.name  << "\">";
			if (x.type == CLOSE_SCOPE)
				out << "</scope>";
			if (x.type == GLOBAL) 
				out << "<global name = \"" << x.name << "\" " << " bitsize=\"" << x.size << "\" />";
			if (x.type == HEADER) 
				out << "<header name = \"" << x.name << "\" " << " bitsize=\"" << x.size * blkcnt << "\" />";
			if (x.type == DATA)
				out << "<data name = \"" << x.name << "\" " << " bitsize=\"" << x.size << "\" />";
		}
		out << "</block_structure>";
	}
};

class BlockBuilder {
private:
	BlockInfoReport rp;
public:
	BlockBuilder();

	void begin_scope(const std::string& name) {
		rp.infolst.emplace_back(BlockInfoReport::OPEN_SCOPE, name);
	}
	void end_scope() {
		assert(rp.infolst.size() > 0);
		rp.infolst.emplace_back(BlockInfoReport::CLOSE_SCOPE);
	}

	// register number start from 1
	unsigned int register_data_block(const std::string& str_info = "");
	// sizes are in bytes
	unsigned int register_summary(size_t global_size, size_t summary_blk_size,
		const std::string& str_info = "");
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

	void clear();
private:
	//std::vector<std::string> info;
	std::vector<unsigned int> summary_sizes, global_sizes;

	FixBlockPtr bptr;
	OBitStream global, summary, blkdata, databuf;

	size_t blkcnt;
	size_t scid, bcid, gcid, last_pos;
	uint64_t start_ptr;

	size_t n_data_block;

	size_t summary_chunk_size;
	bool finish_reg;
};

class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int gid) {
		assert(gid > 0 && gid <= global_ps.size());
		return BitRange(&global, header_size + global_ps[gid - 1], (global_ps[gid] - global_ps[gid - 1]));
	}

	BitRange getSummary(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_ps.size());
		assert(blk < blkcnt);
		size_t stp = blk * summary_chunk_size;
		return BitRange(&summary, (stp + summary_ps[sid - 1])*64, (summary_ps[sid] - summary_ps[sid - 1])*64);
	}

	uint64_t summary_word(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_ps.size());
		assert(blk < blkcnt);
		size_t stp = blk * summary_chunk_size;
		return summary.word(stp + summary_ps[sid - 1]);
	}

	BitRange getData(unsigned int did, size_t blk) {
		assert(did > 0 && did <= str_cnt);
		assert(blk < blkcnt);
		did -= 1;
		if (last_blk != blk) {
			size_t stp = blk * summary_chunk_size;
			stp += summary_chunk_size - 1;
			uint64_t ptrx = summary.word(stp);
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
	BitArray global;
	BitArray summary;
	BitArray data;
	
	size_t blkcnt;
	size_t str_cnt;
private:
	std::string size_report;

	size_t last_blk;
	uint64_t last_ptrx;
	size_t summary_chunk_size, header_size;
	FixBlockPtr bptr;
	std::vector<unsigned int> summary_ps, global_ps;
	friend class BlockBuilder;
};

}//namespace
