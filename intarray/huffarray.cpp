
#include "huffarray.h"
#include "codec/deltacoder.h"

namespace mscds {

static const unsigned int MIN_RATE = 64;

void HuffmanModel::buildModel(std::vector<uint32_t> * data) {
	std::map<uint32_t, unsigned int> cnt;
	for (unsigned int i = 0; i < data->size(); ++i)
		++cnt[(*data)[i]];

	unsigned int n = data->size();

	for (auto it = cnt.begin(); it != cnt.end(); ++it) {
		if (it->second * MIN_RATE > n) {
			freq.push_back(it->first);
			freqset[it->first] = freq.size();
		}
	}
	std::vector<uint32_t> W;
	hc.clear();
	tc.clear();
	if (freq.size() == 0) return ;
	W.reserve(freq.size() + 1);
	W.push_back(0);
	unsigned int sum = 0;
	for (unsigned int i = 0; i < freq.size(); ++i) {
		auto cc = cnt[freq[i]];
		sum += cc;
		W.push_back(cc);
	}
	W[0] = n - sum;
	hc.build(W);
	tc.build(hc);
}

void HuffmanModel::saveModel(OBitStream * out) const {
	out->puts(freq.size(), 32);
	if (freq.size() > 0) {
		for (int i = 0; i < freq.size(); ++i)
			out->puts(freq[i], 32);
		for (int i = 0; i <= freq.size(); ++i)
			out->puts(hc.codelen(i), 16);
	}
}

void HuffmanModel::loadModel(IWBitStream & is, bool decode_only) {
	unsigned int m = is.get(32);
	freq.clear();
	freqset.clear();
	tc.clear();
	hc.clear();
	if (m > 0) {
		for (int i = 0; i < m; ++i) {
			freq.push_back(is.get(32));
			freqset[freq.back()] = freq.size();
		}
		std::vector<uint16_t> L;
		for (int i = 0; i <= m; ++i)
			L.push_back(is.get(16));
		hc.loadCode(m+1, L);
		tc.build(hc);
	}
}

void HuffmanModel::encode(uint32_t val, OBitStream * out) const {
	coder::DeltaCoder dc;
	if (freq.size() > 0) {
		auto it = freqset.find(val);
		if (it != freqset.end()) {
			auto cd = hc.encode(it->second);
			out->puts(cd);
		} else {
			out->puts(hc.encode(0));
			auto cd = dc.encode(val+1);
			out->puts(cd);
		}
	}else {
		auto cd = dc.encode(val+1);
		out->puts(cd);
	}
}

uint32_t HuffmanModel::decode(IWBitStream * is) const {
	coder::DeltaCoder dc;
	unsigned int val = 0;
	if (freq.size() > 0) {
		auto a = tc.decode2(is->peek());
		is->skipw(a.second);
		if (a.first > 0)
			val = freq[a.first - 1];
		else {
			a = dc.decode2(is->peek());
			val = a.first - 1;
			is->skipw(a.second);
		}
	} else {
		auto a = dc.decode2(is->peek());
		val = a.first - 1;
		is->skipw(a.second);
	}
	return val;
}

void HuffmanModel::clear() {
	freq.clear();
	freqset.clear();
	hc.clear();
	tc.clear();
}

//------------------------------------------------------------------------------

void HuffmanArrBuilder::build(HuffmanArray * outds) {
	if (buf.size() > 0) {
		blk.build(&buf, rate1, &out, &opos);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
	out.close();
	bd.build(&(outds->ptr));
	outds->bits = BitArray::create(out.data_ptr(), out.length());
	outds->len = cnt;
	outds->rate1 = rate1;
	outds->rate2 = rate2;
}

void HuffmanArrBuilder::build( OArchive& ar ) {
	HuffmanArray tmp;
	tmp.save(ar);
}


void HuffmanArrBuilder::add(uint32_t val) {
	assert(rate1 > 0 && rate2 > 0);
	buf.push_back(val);
	cnt++;
	if (buf.size() % (rate1 * rate2) == 0) {
		blk.build(&buf, rate1, &out, &opos);
		for (unsigned int i = 0; i < opos.size(); ++i) 
			bd.add(opos[i]);
		buf.clear();
		opos.clear();
	}
}

void HuffmanArrBuilder::clear() {
	buf.clear(); bd.clear(); rate1 = 0; rate2 = 0; opos.clear();
}

void HuffmanArrBuilder::init(unsigned int rate /*= 64*/, unsigned int secondrate/*=512*/) {
	rate1 = rate; rate2 = secondrate;
	cnt = 0;
}

HuffmanArrBuilder::HuffmanArrBuilder() {
	init(64, 512);
}



void HuffmanBlk::mload(const BitArray * enc, const SDArraySml * ptr, unsigned pos) {
	auto st = ptr->prefixsum(pos);
	IWBitStream is(enc->data_ptr(), enc->length(), st);
	model.loadModel(is);
	subsize = is.get(32);
	len = is.get(32);
	this->enc = enc;
	this->ptr = ptr;
	this->blkpos = pos;
}


void HuffmanBlk::build(std::vector<uint32_t> * data, unsigned int subsize, OBitStream * out, std::vector<uint32_t> * opos) {
	clear();
	model.buildModel(data);
	auto cpos = out->length();
	model.saveModel(out);
	this->subsize = subsize;
	out->puts(subsize, 32);
	out->puts(data->size(), 32);

	for (int i = 0; i < data->size(); ++i) {
		if (i % subsize == 0) {
			opos->push_back(out->length() - cpos);
			cpos = out->length();
		}
		uint32_t val = (*data)[i];
		model.encode(val, out);
	}
	if (cpos != out->length())
		opos->push_back(out->length() - cpos);
}

uint32_t HuffmanBlk::lookup(unsigned int i) const {
	assert(i < len);
	auto r = i % subsize;
	auto p = i / subsize;
	auto epos = ptr->prefixsum(p + 1 + blkpos);
	IWBitStream is(enc->data_ptr(), enc->length(), epos);
	
	unsigned int val = 0;
	for (unsigned int j = 0; j <= r; ++j) {
		val = model.decode(&is);
	}
	return val;
}

uint32_t HuffmanArray::lookup(unsigned int i) const {
	assert(i < len);
	auto r = i % (rate1 * rate2);
	auto b = i / (rate1*rate2);
	if (curblk != b) {
		blk.mload(&bits, &ptr, i/rate1 + b);
		curblk = b;
	}
	return blk.lookup(r);
}

void HuffmanArray::getEnum(unsigned int pos, Enum * e) const {
	assert(pos < len);
	e->pos = pos;
	auto r = pos % (rate1 * rate2);
	auto b = pos / (rate1*rate2);
	auto blkst = pos/rate1 + b;
	blk.mload(&bits, &ptr, blkst);
	auto i = r;
	r = i % rate1;
	auto p = i / rate1;
	auto epos = ptr.prefixsum(p + 1 + blkst);
	e->is.init(bits.data_ptr(), bits.length(), epos);
	for (unsigned int j = 0; j < r; ++j)
		blk.getModel().decode(&(e->is));
}

uint64_t HuffmanArray::Enum::next() {
	auto val = blk.getModel().decode(&is);
	++pos;
	if (pos < data->len) {
		auto bs = data->rate1 * data->rate2;
		if (pos % bs == 0) {
			auto b = pos / bs;
			auto blkst = pos/data->rate1 + b;
			blk.mload(&(data->bits), &(data->ptr), blkst);
			auto epos = data->ptr.prefixsum(1 + blkst);
			is.init(data->bits.data_ptr(), data->bits.length(), epos);
		}
	}
	return val;
}


void HuffmanBlk::clear() {
	enc = NULL;
	ptr = NULL;
	subsize = 0;
	len = 0;
	blkpos = 0;
	model.clear();
}

const HuffmanModel& HuffmanBlk::getModel() const {
	return model;
}

void HuffmanArray::save(OArchive& ar) const {
	ar.startclass("huffman_code", 1);
	ar.var("length").save(len);
	ar.var("sample_rate").save(rate1);
	ar.var("bigblock_rate").save(rate2);
	ptr.save(ar.var("pointers"));
	bits.save(ar.var("bits"));
	ar.endclass();
}

void HuffmanArray::load(IArchive& ar) {
	ar.loadclass("huffman_code");
	ar.var("length").load(len);
	ar.var("sample_rate").load(rate1);
	ar.var("bigblock_rate").load(rate2);
	ptr.load(ar.var("pointers"));
	bits.load(ar.var("bits"));
	ar.endclass();
	curblk = -1;
}

void HuffmanArray::clear() {
	curblk = -1;
	bits.clear();
	ptr.clear();
	len = 0;
	rate1 = 0;
	rate2 = 0;
}

uint64_t HuffmanArray::length() const {
	return len;
}

}
