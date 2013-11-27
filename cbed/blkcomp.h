#pragma once

#include "utils/param.h"
#include "framework/archive.h"
#include "utils/cache_table.h"
#include "extcodec/mem_codec.h"
#include "intarray/sdarray_sml.h"
#include "bitarray/bitstream.h"

#include <string>
#include <vector>
#include <sstream>

namespace mscds {

class BlkCompBuilder;
class BlkCompQuery;

class LineBlockBuilder {
public:
	void add(const std::string& line);
	void build();
	void clear();
private:
	std::string blkmem;
	friend class BlkCompBuilder;
};

class LineBlock {
public:
	std::string getline(unsigned int i) const;
	size_t size() const;
	void clear();
private:
	void post_load();
	void _buildptr();

	size_t _size;
	std::string blkmem;
	std::vector<unsigned int> lineptr;

	friend class BlkCompQuery;
};

class BlkCompQuery;

class BlkCompBuilder {
public:
	BlkCompBuilder() { init(); }
	void init(Config* conf);
	void init();
	void add(const std::string& line);
	void build(BlkCompQuery* data);
	void build(mscds::OArchive& ar);

	void clear();
private:
	void flush_data();

	OByteStream os;
	std::string buff;

	SDArraySmlBuilder pbd;
	SnappyCodec codec;
	LineBlockBuilder bd;
	unsigned int maxblksz, curblksz, entcnt, blkcnt;
};

class BlkCompQuery {
public:
	BlkCompQuery();
	void init(); // default cache size = 32
	void init(Config* conf) {}
	void init(unsigned int cache_size, Config* conf) {}
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	std::string getline(unsigned int i) const;
	void clear();
public:
	class Enum : public mscds::EnumeratorInt<std::string> {
	public:
		Enum() {}
		bool hasNext() const;
		std::string next();
	private:
		friend class BlkCompQuery;
		LineBlock blkdata;
		unsigned int idx, cblk, bidx;
		const BlkCompQuery * parent;
	};
	void getEnum(unsigned int idx, Enum * e) const;
private:
	const LineBlock& getblk(unsigned int b) const;
	void load_blk(unsigned int blk, LineBlock& lnblk) const;

	unsigned int maxblksz;
	size_t entcnt;
	SDArraySml bptr;
	BitArray bits;

	void prepare_ptr();
	const char * ptr;
	size_t len;

	SnappyCodec codec;

	mutable std::vector<LineBlock> cache;
	mutable utils::LRU_Policy cache_mamager;

	mutable int lastblk;
	mutable unsigned int lastanswer;
	friend class BlkCompBuilder;
};

}//namespace
