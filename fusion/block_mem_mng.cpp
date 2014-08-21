#include "block_mem_mng.h"

#include <sstream>

namespace mscds {

BlockBuilder::BlockBuilder(): blkcnt(0), finish_reg(false) {
	n_data_block = 0;
}

unsigned int BlockBuilder::register_data_block(const std::string& str_info) {
	rp.datainfoloc.push_back(rp.infolst.size());
	rp.infolst.emplace_back(BlockInfoReport::DATA, str_info);
	++n_data_block;
	return n_data_block;
}

unsigned int BlockBuilder::register_summary(size_t global_size, size_t summary_blk_size, 
		const std::string& str_info) {
	assert(!finish_reg);
	summary_acc.declare_segment((summary_blk_size + 7) / 8);
	global_acc.declare_segment(global_size);

	rp.infolst.emplace_back(BlockInfoReport::GLOBAL, str_info, global_size * 8);
	rp.infolst.emplace_back(BlockInfoReport::HEADER, str_info, summary_blk_size * 64);
	return global_acc.count();
}

std::pair<unsigned int, unsigned int> BlockBuilder::current_reg_numbers() {
	return std::pair<unsigned int, unsigned int>(n_data_block, global_acc.count());
}

void BlockBuilder::init_data() {
	//initialize variables
	finish_reg = true; start_ptr = 0;
	summary_acc.declare_segment(1); // 1 word for pointer

	bptr.init(n_data_block);
	//save array

	OBitStream buf;
	global_acc.store_context(buf);
	summary_acc.store_context(buf);
	global_acc.init_block();
	summary_acc.init_block();
	bcid = 0;
	scid = 0;
	gcid = 0;

	//save global here
	size_t header_size = buf.length() / 8;
	assert(header_size < (1<<16));
	global.puts(header_size, 16);
	buf.close();
	global.append(buf);

	rp.header_overhead = 64;
}

void BlockBuilder::start_block() {
}

void BlockBuilder::set_global(unsigned int sid) {
	global_acc.add_data(global, sid);
	gcid++;
}

void BlockBuilder::set_global(unsigned int sid, const ByteMemRange& r) {
	global_acc.add_data(global, sid, r);
	gcid++;
}

void BlockBuilder::set_global(unsigned int sid, const OBitStream& os) {
	global_acc.add_data(global, sid, os);
	gcid++;
}

void BlockBuilder::set_summary(unsigned int sid) {
	summary_acc.add_data(summary, sid);
	scid++;
}

void BlockBuilder::set_summary(unsigned int sid, const ByteMemRange& r) {
	summary_acc.add_data(summary, sid, r);
	scid++;
}

OBitStream& BlockBuilder::start_data(unsigned int did) {
	assert(did == bcid + 1);
	last_data_pos = databuf.length();
	return databuf;
}

void BlockBuilder::end_data() {
	size_t sz = databuf.length() - last_data_pos;
	rp.set_data_size(bcid+1, sz);
	bptr.add(sz);
	last_data_pos = databuf.length();
	bcid++;
}

void BlockBuilder::end_block() {
	assert(scid + 1 == summary_acc.count());
	assert(bcid == n_data_block);
	databuf.close();
	
	auto cp = blkdata.length();
	bptr.saveBlock(&blkdata);
	#ifdef __REPORT_FUSION_BLOCK_SIZE__
		rp.block_overhead += blkdata.length() - cp;
	#endif

	blkdata.append(databuf);
	databuf.clear();

	summary_acc.add_data(summary, scid + 1, ByteMemRange::ref(start_ptr));
	start_ptr = blkdata.length();

	bcid = 0;
	scid = 0;
	global_acc.init_block();
	summary_acc.init_block();
	blkcnt++;
	bptr.reset();
}

void BlockBuilder::build(BlockMemManager *mng) {
	assert(gcid == global_acc.count());
	//if (global.length() != (header_size + global_struct_size) * 8)
	//	throw std::runtime_error("size mismatch");
	rp.blkcnt = blkcnt;
	summary.close();
	global.build(&mng->global_bits);
	summary.build(&mng->summary_bits);
	blkdata.build(&mng->data_bits);
	mng->blkcnt = blkcnt;
	mng->str_cnt = n_data_block;
	mng->init();
	std::stringstream ss;
	rp.report(ss);
	mng->size_report = ss.str();
}

void BlockBuilder::clear() {
	start_ptr = 0;
	blkcnt = 0;
	finish_reg = false;
	n_data_block = 0;
	bcid = 0;
	last_data_pos = 0;
	bptr.clear();
	rp.clear();

	scid = gcid = 0;
}

//-----------------------------------------------------------------------------------------------

void BlockMemManager::init() {
	std::vector<unsigned int> summary_sizes, global_sizes;
	IWBitStream is(global_bits);
	//VByteArray::load(is, summary_sizes);
	unsigned int n = str_cnt;
	header_size = is.get(16);
	size_t p = 16;
	p += global_acc.load_context(global_bits, p);
	p += summary_acc.load_context(global_bits, p);
	header_size += 2;
	assert((header_size * 8) == p);
	header_size *= 8; // bytes --> bits

	bptr.init(n);

	last_blk = ~0ULL;
	last_ptrx = ~0ULL;
}

void BlockMemManager::save(mscds::OutArchive &ar) const {
	ar.startclass("fusion_block_manager", 1);
	ar.var("struct_count").save(str_cnt);
	ar.var("block_count").save(blkcnt);
	global_bits.save(ar.var("global"));
	summary_bits.save(ar.var("summary_data"));
	data_bits.save(ar.var("data"));

	if (!size_report.empty()) {
		ar.annotate(size_report);
	}
	ar.endclass();
}

void BlockMemManager::load(mscds::InpArchive &ar) {
	ar.loadclass("fusion_block_manager");
	ar.var("struct_count").load(str_cnt);
	ar.var("block_count").load(blkcnt);
	global_bits.load(ar.var("global"));
	summary_bits.load(ar.var("summary_data"));
	data_bits.load(ar.var("data"));
	ar.endclass();
	init();
}

void BlockMemManager::clear() {
	bptr.clear();
	summary_acc.clear();
	global_acc.clear();
	summary_bits.clear();
	global_bits.clear();
	data_bits.clear();

	blkcnt = 0; str_cnt = 0;
}

void BlockMemManager::inspect(const std::string &cmd, std::ostream &out) {
	out << "{";
	out << "\"struct_count\": " << str_cnt << ", ";
	out << "\"block_count\": " << blkcnt << ", ";

	out << "\"summary_ptr_bit_size\": " << 64 << ", ";

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


}//namespace

