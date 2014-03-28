#include "blkgroup_array.h"

namespace mscds {

void BlockMemManager::init() {
	std::vector<unsigned int> summary_sizes, global_sizes;
	IWBitStream is(summary);
	//VByteArray::load(is, summary_sizes);
	unsigned int n = 0;
	n = VByteArray::extract(is);
	assert(str_cnt == n);
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

	last_blk =  ~0ULL;
	last_ptrx = ~0ULL;
}

unsigned int BlockBuilder::register_data_block() {
	return ++n_data_block;
}

unsigned int BlockBuilder::register_summary(size_t global_size, size_t summary_blk_size, const std::string& str_info) {
	summary_sizes.push_back(summary_blk_size);
	global_sizes.push_back(global_size);
	summary_chunk_size += summary_blk_size;
	info.push_back(str_info);
	return summary_sizes.size();
}

void BlockBuilder::init_data() {
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
	bcid = 0;  scid = 0;  gcid = 0;
}

void BlockBuilder::start_block() {}

void BlockBuilder::set_global(unsigned int sid) {
	assert(sid == gcid + 1);
	if (global_sizes[gcid] > 0)
		header.put0(8*global_sizes[gcid]);
	gcid++;
}

void BlockBuilder::set_global(unsigned int sid, const MemRange& r) {
	assert(sid == gcid + 1);
	assert(r.len <= global_sizes[gcid]);
	header.puts_c(r.ptr, r.len);
	if (r.len < global_sizes[gcid])
		header.put0(8*(global_sizes[gcid] - r.len));
	gcid++;
}

void BlockBuilder::set_summary(unsigned int sid) {
	assert(sid == scid + 1);
	if (summary_sizes[scid] > 0)
		summary.put0(8*summary_sizes[scid]);
	scid++;
}

void BlockBuilder::set_summary(unsigned int sid, const MemRange& r) {
	assert(sid == scid + 1);
	assert(r.len <= summary_sizes[scid]);
	summary.puts_c(r.ptr, r.len);
	if (r.len < summary_sizes[scid])
		summary.put0(8*(summary_sizes[scid] - r.len));
	scid++;
}

OBitStream& BlockBuilder::start_data(unsigned int did) {
	assert(did == bcid + 1);
	last_pos = buffer.length();
	return buffer;
}

void BlockBuilder::end_data() {
	bptr.set(bcid, buffer.length() - last_pos);
	last_pos = buffer.length();
	bcid++;
}

void BlockBuilder::end_block() {
	assert(scid == summary_sizes.size());
	assert(bcid == n_data_block);
	buffer.close();
	bptr.saveBlock(&data);
	data.append(buffer);
	buffer.clear();

	summary.puts_c((const char*)&start_ptr, sizeof(start_ptr));
	start_ptr = data.length();

	scid = 0; bcid = 0;
	blkcnt++;
	bptr.reset();
}

void BlockBuilder::build(BlockMemManager *mng) {
	assert(gcid == global_sizes.size());
	if (header.length() != (header_size + global_struct_size) * 8)
		throw std::runtime_error("size mismatch");
	summary.close();
	header.append(summary);
	header.build(&mng->summary);
	data.build(&mng->data);
	mng->blkcnt = blkcnt;
	mng->info = info;
	mng->str_cnt = summary_sizes.size();
	mng->init();
}

BlockBuilder::BlockBuilder() : blkcnt(0), finish_reg(false) {
	summary_chunk_size = 0;
	n_data_block = 0;
}

void BlockBuilder::clear() {
	start_ptr = 0;
	blkcnt = 0;
	finish_reg = false;
	summary_chunk_size = 0;
	global_struct_size = 0;
	header_size = 0;
	n_data_block = 0;
	scid = 0;
	gcid = 0;
	bcid = 0;
	last_pos = 0;
	info.clear();
	summary_sizes.clear();
	global_sizes.clear();
	bptr.clear();
}

}//namespace

