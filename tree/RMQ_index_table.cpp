
#include "RMQ_index_table.h"

namespace mscds {

void RMQ_index_blk::build(const std::vector<int> &wordvals, unsigned int blksize, bool _min_struct, mscds::RMQ_index_blk *out) {
	assert(blksize > 0 && (blksize & (blksize - 1)) == 0); // blksize must be in power of 2
	OBitStream subblkv;
	std::vector<int> sbv, bv;
	unsigned int sblcnt = 0;
	int mx;
	if (_min_struct) mx = std::numeric_limits<int>::max();
	else mx = std::numeric_limits<int>::min();
	sbv.reserve(blksize);
	for (size_t i = 0; i < wordvals.size(); ++i) {
		sbv.push_back(wordvals[i]);
		if (_min_struct) mx = std::min(mx, wordvals[i]);
		else mx = std::max(mx, wordvals[i]);
		if ((i + 1) % blksize == 0 && i > 0) {
			RMQ_table_access::build_stream(sbv, _min_struct, &subblkv);
			bv.push_back(mx);
			sbv.clear();
			if (_min_struct) mx = std::numeric_limits<int>::max();
			else mx = std::numeric_limits<int>::min();
			sblcnt++;
		}
	}
	if (sbv.size() > 0) {
		assert(sbv.size() < blksize);
		for (size_t i = sbv.size(); i < blksize; ++i)
			sbv.push_back(0);
		RMQ_table_access::build_stream(sbv, _min_struct, &subblkv);
		bv.push_back(mx);
		sblcnt++;
	}
	RMQ_table_access::build(bv, _min_struct, &(out->blockrmq));
	RMQ_table_access::build_stream(sbv, _min_struct, &subblkv);
	subblkv.close();
	BitArray bt = BitArray::create(subblkv.data_ptr(), subblkv.length());
	out->subblkrmq.init_shared(sblcnt, blksize, 0, bt);

	out->len = wordvals.size();
	out->blksize = blksize;
}

void RMQ_table_access::build(const std::vector<int> &values, bool _min_struct, RMQ_table_access *tbl) {
	OBitStream out;
	build_stream(values, _min_struct, &out);
	out.close();
	tbl->init(values.size(), BitArray::create(out.data_ptr(), out.length()));
}

size_t RMQ_table_access::build_stream(const std::vector<int> &values, bool _min_struct, OBitStream *out) {
	size_t len = values.size();
	size_t layerspan = 2;
	size_t bitsize = 0;
	unsigned int layer = 1;
	std::vector<unsigned int> prelayer(values.size(), 0);
	while (layerspan <= len) {
		for (size_t i = 0; i <= len - layerspan; ++i) {
			unsigned int ridx = prelayer[i];
			size_t j = i + layerspan/2;
			if (_min_struct) {
				if (values[i + prelayer[i]] > values[j + prelayer[j]])
					ridx = layerspan / 2 + prelayer[j];
			} else {
				if (values[i + prelayer[i]] < values[j + prelayer[j]])
					ridx = layerspan / 2 + prelayer[j];
			}
			prelayer[i] = ridx;
			out->puts(ridx, layer);
			bitsize += layer;
		}
		layer += 1;
		layerspan *= 2;
	}
	return bitsize;
}

size_t RMQ_table_access::build_start(size_t len, std::vector<unsigned int> *starts) {
	starts->clear();
	size_t layerspan = 2;
	unsigned int layer = 1;
	starts->push_back(0);
	while (layerspan <= len) {
		starts->push_back(layer * (len - layerspan + 1));
		layer += 1;
		layerspan *= 2;
	}
	for (size_t i = 1; i < starts->size(); ++i) {
		(*starts)[i] += (*starts)[i - 1];
	}
	return starts->back();
}

void RMQ_table_access::init_shared(unsigned int nblk, size_t seqlen, size_t base_pos, BitArray b) {
	this->nblk = nblk;
	len = seqlen;
	this->start_pos = base_pos;
	this->base_pos = base_pos;
	this->bits = b;
	bit_size = build_start(seqlen, &starts);
	if (bit_size + start_pos > b.length()) throw std::runtime_error("over flow");
}

void RMQ_table_access::init(size_t seqlen, BitArray b) {
	start_pos = 0;
	base_pos = 0;
	nblk = 1;
	bits = b;
	this->len = seqlen;
	bit_size = build_start(seqlen, &starts);
}

}
