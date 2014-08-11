#include "blkgroup_array.h"

namespace mscds {

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

std::pair<unsigned int, unsigned int> BlockBuilder::current_reg_numbers() {
	return std::pair<unsigned int, unsigned int>(n_data_block, summary_sizes.size());
}

void BlockBuilder::init_data() {
	//initialize variables
	finish_reg = true; start_ptr = 0;
	assert(summary_sizes.size() < 128);
	bptr.init(summary_sizes.size());
	//save array
	assert(global_sizes.size() == summary_sizes.size());
	VByteStream::append(header, global_sizes.size());
	for (unsigned int i = 0; i < global_sizes.size(); ++i)
		VByteStream::append(header, global_sizes[i]);
	for (unsigned int i = 0; i < summary_sizes.size(); ++i)
		VByteStream::append(header, summary_sizes[i]);
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

void BlockBuilder::set_global(unsigned int sid, const OBitStream& os) {
	assert(sid == gcid + 1);
	assert(os.length() <= global_sizes[gcid] * 8);
	header.append(os);
	if (os.length() < global_sizes[gcid] * 8)
		header.put0(8*(global_sizes[gcid] * 8 - os.length()));
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
	bptr.add(buffer.length() - last_pos);
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

//------------------------------------------------------------------------

void BlockMemManager::init() {
	std::vector<unsigned int> summary_sizes, global_sizes;
	IWBitStream is(summary);
	//VByteArray::load(is, summary_sizes);
	unsigned int n = 0;
	n = VByteStream::extract(is);
	assert(str_cnt == n);
	assert(n < 128);
	for (unsigned int i = 0; i < n; ++i)
		global_sizes.push_back(VByteStream::extract(is));
	for (unsigned int i = 0; i < n; ++i)
		summary_sizes.push_back(VByteStream::extract(is));
	header_size = is.get(16);
	header_size += 2;
	//assert((header_size * 8) == is.extracted());
	
	bptr.init(n);
	summary_ps = prefixsum_vec(summary_sizes);
	global_ps = prefixsum_vec(global_sizes);
	global_struct_size = global_ps.back();
	summary_chunk_size = summary_ps.back() + sizeof(uint64_t);

	header_size *= 8; // bytes --> bits
	global_struct_size *= 8;
	summary_chunk_size *= 8;
	prefix_size = global_struct_size + header_size;

	last_blk = ~0ULL;
	last_ptrx = ~0ULL;

	for (unsigned int i = 0; i < summary_ps.size(); ++i)
		summary_ps[i] *= 8;
	for (unsigned int i = 0; i < summary_ps.size(); ++i)
		global_ps[i] *= 8;
}

void BlockMemManager::save(mscds::OutArchive &ar) const {
	ar.startclass("fusion_block_manager", 1);
	ar.var("struct_count").save(str_cnt);
	ar.var("block_count").save(blkcnt);
	summary.save(ar.var("summary_data"));
	std::string s = std::accumulate(info.begin(), info.end(), std::string());
	data.save(ar.var("data" + s));
	ar.endclass();
}

void BlockMemManager::load(mscds::InpArchive &ar) {
	ar.loadclass("fusion_block_manager");
	ar.var("struct_count").load(str_cnt);
	ar.var("block_count").load(blkcnt);
	summary.load(ar.var("summary_data"));
	//std::string s = std::accumulate(info.begin(), info.end(), std::string());
	data.load(ar);
	ar.endclass();
	init();
}

void BlockMemManager::clear() {
	bptr.clear();
	global_ps.clear(); summary_ps.clear();
	summary.clear(); data.clear();
	info.clear();

	blkcnt = 0; str_cnt = 0;
}

void BlockMemManager::inspect(const std::string &cmd, std::ostream &out) {
	out << "{";
	out << "\"struct_count\": " << str_cnt << ", ";
	out << "\"block_count\": " << blkcnt << ", ";

	out << "\"summary_ptr_bit_size\": " << 64 << ", ";

	out << "\"global_bit_sizes\": [";
	for (size_t i = 1; i < global_ps.size(); ++i) {
		if (i != 1) out << ", ";
		out << (global_ps[i] - global_ps[i-1]);
	}
	out << "], ";

	out << "\"summary_bit_sizes\": [";
	for (size_t i = 1; i < global_ps.size(); ++i) {
		if (i != 1) out << ", ";
		out << (summary_ps[i] - summary_ps[i-1]);
	}
	out << "], ";
	std::vector<size_t> bsz(str_cnt + 1, 0);
	for (size_t i = 0; i < blkcnt; ++i) {
		for (size_t j = 1; j <= str_cnt; ++j) {
			auto x = getData(j, i);
			bsz[j] += x.len;
			bsz[0] += bptr.ptr_space();
		}
	}
	out << "\"struct_block_bit_sizes\": [";
	for (size_t i = 0; i < bsz.size(); ++i) {
		if (i != 0) out << ", ";
		out << bsz[i];
	}
	out << "]";
	out << "}";
}

std::vector<unsigned int> BlockMemManager::prefixsum_vec(const std::vector<unsigned int> &v) {
	std::vector<unsigned int> out(v.size() + 1);
	out[0] = 0;
	for (size_t i = 1; i <= v.size(); ++i)
		out[i] = out[i - 1] + v[i - 1];
	return out;
}


//------------------------------------------------------------------------

void StructIDList::addId(const std::string &name) {
	_lst.push_back(-1);
}

void StructIDList::checkId(const std::string &name) {
	//int v = _lst.front(); _lst.pop_front();
	int v = _lst[pfront];
	pfront++;
	if (v != -1) throw std::runtime_error("failed");
}

void StructIDList::add(unsigned int id) {
	_lst.push_back((int)id);
}

unsigned int StructIDList::get() {
	//int v = (int)_lst.front(); _lst.pop_front();
	int v = _lst[pfront];
	pfront++;
	assert(v > 0);
	return (unsigned int)v;
}

void StructIDList::save(mscds::OutArchive &ar) const {
	ar.startclass("block_struct_list", 1);
	uint32_t len = _lst.size();
	ar.var("len").save(len);
	ar.var("list");
	for (unsigned int i = 0; i < len; ++i) {
		int16_t v = _lst[i];
		ar.save_bin(&v, sizeof(v));
	}
	ar.endclass();
}

void StructIDList::load(mscds::InpArchive &ar) {
	int class_version = ar.loadclass("block_struct_list");
	uint32_t len = 0;
	ar.var("len").load(len);
	ar.var("list");
	for (unsigned int i = 0; i < len; ++i) {
		int16_t v = 0;
		ar.load_bin(&v, sizeof(v));
		_lst.push_back(v);
	}
	ar.endclass();
}

void StructIDList::clear() {
	_lst.clear();
	pfront = 0;
}

void StructIDList::reset() {
	pfront = 0;
}

}//namespace

