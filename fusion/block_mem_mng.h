#pragma once

/** \file

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
#include <memory>

namespace mscds {

/// array of bytes
class ByteMemRange {
	std::shared_ptr<char> _ptr;
	unsigned int len;
	static void do_nothing_deleter(char*) {}
	static void array_deleter(char* p) {
		delete[] p;
	}
public:
	ByteMemRange(): len(0) {}
	ByteMemRange(ByteMemRange& m): _ptr(m._ptr), len(m.len) {}
	ByteMemRange(ByteMemRange&& m): _ptr(std::move(m._ptr)), len(m.len) {}

	template<typename T>
	static ByteMemRange ref(T& t, size_t len) {
		assert(len <= sizeof(T));
		ByteMemRange ret;
		ret._ptr = std::shared_ptr<char>((char*)(&t), do_nothing_deleter);
		ret.len = len;
		return ret;
	}
	template<typename T>
	static ByteMemRange ref(T& t) {
		return ref<T>(t, sizeof(t));
	}

	template<typename T>
	static ByteMemRange val(T v, size_t len) {
		char * p = new char[len];
		*((T*)p) = v;
		ByteMemRange ret;
		ret._ptr = std::shared_ptr<char>(p, array_deleter);
		ret.len = len;
		return ret;
	}
	template<typename T>
	static ByteMemRange val(T v) {
		return val(v, sizeof(v));
	}

	static ByteMemRange val_c(char v) {
		return val<char>(v, 1);
	}

	size_t length() const {
		return len;
	}

	char* ptr() {
		return _ptr.get();
	}
	const char* ptr() const {
		return _ptr.get();
	}

};


// sub-range of a BitArray
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

/// build fixed sized interleaved arrays. See FixedSizeMemAccess
template<unsigned UNIT_BIT_SIZE>
class FixedSizeMemAccBuilder {
public:
	FixedSizeMemAccBuilder(): blkcnt(0), cur(0) {}
	/// declare a segment. The length of the segment is (`size' * UNIT_BIT_SIZE) bits.
	unsigned int declare_segment(size_t size) {
		_sizes.push_back(size);
		return _sizes.size();
	}

	/// returns number of segments
	unsigned int count() const {
		return _sizes.size();
	}

	/// start adding data
	void init_block() {
		assert(_sizes.size() < 128);
		cur = 0;
		blkcnt += 1;
	}

	/// add nothing, (auto fill up the space with 0)
	void add_data(mscds::OBitStream& buf, int sid) {
		auto s = check_block_data(sid, 0);
		if (s > 0) buf.put0(s);
	}

	/// add array of bytes
	void add_data(mscds::OBitStream& buf, int sid, const ByteMemRange& r) {
		auto s = check_block_data(sid, r.length() * 8);
		buf.puts_c(r.ptr(), r.length());
		if (s > 0) buf.put0(s);
	}

	/// add bits
	void add_data(mscds::OBitStream& buf, int sid, const OBitStream& o) {
		auto s = check_block_data(sid, o.length());
		buf.append(o);
		if (s > 0) buf.put0(s);
	}

	/// unit size
	uint8_t unit_bit_size() const {
		return UNIT_BIT_SIZE;
	}

	/// stores the structure's sizes to buffer
	void store_context(mscds::OBitStream& buf) {
		assert(_sizes.size() < 128);
		VByteStream::append(buf, UNIT_BIT_SIZE);
		VByteStream::append(buf, _sizes.size());
		VByteStream::append(buf, blkcnt);
		for (unsigned int i = 0; i < _sizes.size(); ++i)
			VByteStream::append(buf, _sizes[i]);
	}

	void clear() { _sizes.clear(); cur = 0; blkcnt = 0; }

	/// check the added size for overflow
	/** 
	The current assumption is block data is added in the order of the id value
	*/
	unsigned int check_block_data(unsigned int id, unsigned int bit_size) {
		assert(id == cur + 1);
		assert(_sizes[cur]*UNIT_BIT_SIZE >= bit_size);
		cur++;
		return _sizes[cur - 1]*UNIT_BIT_SIZE - bit_size;
	}

private:
	std::vector<unsigned int> _sizes;
	unsigned int cur;
	unsigned int blkcnt;
};

/// fixed sized interleaved arrays (i.e. Array of fixed size blocks. Each block contains some fixed size segments.)
/** 
The start of each block is aligned by UNIT_BIT_SIZE (should be 8, 16, 32, 64)
*/
template<unsigned UNIT_BIT_SIZE>
class FixedSizeMemAccess {
public:
	/// load block sizes from buffer
	size_t load_context(BitArray& b, size_t pos = 0) {
		size_t ipos = pos;
		size_t ubs = VByteStream::extract(b, ipos);
		assert(UNIT_BIT_SIZE == ubs);
		size_t n = VByteStream::extract(b, ipos);
		blkcnt = VByteStream::extract(b, ipos);
		for (unsigned int i = 0; i < n; ++i)
			_sizes.push_back(VByteStream::extract(b, ipos));
		ps_sz.resize(_sizes.size() + 1);
		ps_sz[0] = 0;
		for (size_t i = 1; i <= _sizes.size(); ++i)
			ps_sz[i] = ps_sz[i - 1] + _sizes[i - 1];
		_chunk_size = ps_sz.back();
		return ipos - pos;
	}

	/// the number of segments
	size_t count() const { return _sizes.size(); }

	/// the number of blocks
	size_t length() const { return blkcnt; }

	void clear() { _sizes.clear(); ps_sz.clear(); _chunk_size = 0; blkcnt = 0; }

	
	uint8_t unit_bit_size() const {
		return UNIT_BIT_SIZE;
	}

	/// get the data range of segment `id' in block `index'
	BitRange get_range(unsigned id, unsigned index, BitArray * a, unsigned int st = 0) {
		std::pair<unsigned int, unsigned int> p = _get_loc_range(id, index);
		return BitRange(a, st + p.first* unit_bit_size(), p.second * unit_bit_size());
	}

	/// (internal use only) get start of segment `id' in block `index'
	size_t _get_start(unsigned int id, unsigned int index) {
		size_t stp = index * _chunk_size;
		return stp + ps_sz[id - 1];
	}

	/// (internal use only) get start and length of segment `id' in block `index'
	std::pair<unsigned int, unsigned int> _get_loc_range(unsigned int id, unsigned int index) {
		size_t stp = index * _chunk_size;
		return std::pair<unsigned int, unsigned int>(stp + ps_sz[id - 1], ps_sz[id] - ps_sz[id - 1]);
	}

private:
	std::vector<unsigned int> _sizes, ps_sz;
	size_t blkcnt;
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

/// build block
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
	void set_global(unsigned int sid, const ByteMemRange& r);
	void set_global(unsigned int sid, const OBitStream& r);

	void start_block();

	void set_summary(unsigned int sid);
	void set_summary(unsigned int sid, const ByteMemRange& r);

	OBitStream& start_data(unsigned int did);
	void end_data();

	void end_block();

	void build(BlockMemManager* mng);

	void clear();
private:
	FixBlockPtr bptr;
	FixedSizeMemAccBuilder<8> global_acc;
	FixedSizeMemAccBuilder<64> summary_acc;
	OBitStream global, summary, blkdata, databuf;

	size_t blkcnt;
	size_t n_data_block;

	size_t gcid, scid, bcid;
	size_t start_ptr, last_data_pos;
	bool finish_reg;
};

/// block manager
class BlockMemManager {
public:
	size_t blkCount() const { return blkcnt; }

	BitRange getGlobal(unsigned int gid) {
		assert(gid > 0 && gid <= global_acc.count());
		//auto p = global_acc.get_data_range(gid, 0); // only 1 block
		//return BitRange(&global_bits, header_size + p.first * global_acc.unit_bit_size(), p.second * global_acc.unit_bit_size());
		return global_acc.get_range(gid, 0, &global_bits, header_size);
	}

	BitRange getSummary(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_acc.count());
		assert(blk < blkcnt);
		//auto p = summary_acc.get_data_range(sid, blk);
		//return BitRange(&summary_bits, p.first*summary_acc.unit_bit_size(), p.second*summary_acc.unit_bit_size());
		return summary_acc.get_range(sid, blk, &summary_bits, 0);
	}

	uint64_t summary_word(unsigned int sid, size_t blk) {
		assert(sid > 0 && sid <= summary_acc.count());
		assert(blk < blkcnt);
		return summary_bits.word(summary_acc._get_start(sid, blk));
	}

	BitRange getData(unsigned int did, size_t blk) {
		assert(did > 0 && did <= str_cnt);
		assert(blk < blkcnt);
		did -= 1;
		if (last_blk != blk) {
			auto px = summary_acc._get_start(summary_acc.count(), blk);
			uint64_t ptrx = summary_bits.word(px); // last structure is the pointer
			bptr.loadBlock(data_bits, ptrx, 0);
			last_blk = blk;
			last_ptrx = ptrx;
		}
		size_t base = last_ptrx + bptr.ptr_space();
		return BitRange(&data_bits, base + bptr.start(did), bptr.length(did));
	}

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();
	void inspect(const std::string &cmd, std::ostream &out);

private:
	static std::vector<unsigned int> prefixsum_vec(const std::vector<unsigned int>& v);
	void init();
	
private:   //essensial data
	FixedSizeMemAccess<8> global_acc;
	FixedSizeMemAccess<64> summary_acc;

	BitArray global_bits;
	BitArray summary_bits;
	BitArray data_bits;
	
	size_t blkcnt;
	size_t str_cnt;
private:
	std::string size_report;

	size_t last_blk, header_size;
	uint64_t last_ptrx;
	FixBlockPtr bptr;
	friend class BlockBuilder;
};

}//namespace
