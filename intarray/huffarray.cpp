
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
}
