
#include "huffarray.h"
#include "codec/deltacoder.h"

namespace mscds {

static const unsigned int MIN_RATE = 95;

void HuffmanModel::buildModel_old(const std::vector<uint32_t> * data) {
	freq.clear();
	freqset.clear();
	std::unordered_map<uint32_t, unsigned int> cnt;
	for (unsigned int i = 0; i < data->size(); ++i)
		++cnt[(*data)[i]];

	unsigned int n = data->size();
	for (auto it = cnt.begin(); it != cnt.end(); ++it) {
		if (it->second * MIN_RATE > n) {
			freq.push_back(it->first);
			freqset[it->first] = freq.size();
		}
	}
	//-------------------------
	std::vector<uint32_t> W;
	hc.clear();
	tc.clear();
	if (freq.size() == 0) return ;
	W.reserve(freq.size() + 1);
	W.push_back(0);
	unsigned int sum = 0;
	for (unsigned int i = 0; i < freq.size(); ++i) {
		auto cc = cnt.at(freq[i]);
		sum += cc;
		W.push_back(cc);
	}
	W[0] = n - sum;
	hc.build(W);
	tc.build(hc);
}


void HuffmanModel::buildModel_cnt(unsigned int n,
		const std::unordered_map<uint32_t, unsigned int> & cnt,
		unsigned int max_symbol_size) {
	freq.clear();
	freqset.clear();

	std::vector<std::pair<unsigned int, uint32_t> > sfreq;
	for (auto it = cnt.begin(); it != cnt.end(); ++it)
		sfreq.push_back(std::make_pair(it->second, it->first));
	std::sort(sfreq.begin(), sfreq.end(), std::greater<std::pair<unsigned int, uint32_t> >());

	std::map<uint32_t, uint32_t> remapt;
	max_symbol_size = std::max<unsigned int>(max_symbol_size, 8u);
	unsigned rmsize = std::min<unsigned int>(max_symbol_size, sfreq.size());
	for (unsigned int i = 0; i < rmsize; ++i) {
		if (sfreq[i].first == 1) break;
		auto val = sfreq[i].second;
		freq.push_back(val);
		freqset[val] = freq.size();
	}
	//-------------------------
	std::vector<uint32_t> W;
	hc.clear();
	tc.clear();
	if (freq.size() == 0) return;
	W.reserve(freq.size() + 1);
	W.push_back(0);
	unsigned int sum = 0;
	for (unsigned int i = 0; i < freq.size(); ++i) {
		unsigned int cc = cnt.at(freq[i]);
		sum += cc;
		W.push_back(cc);
	}
	W[0] = n - sum;
	hc.build(W);
	tc.build(hc);
}

void HuffmanModel::buildModel(const std::vector<uint32_t> * data, const Config* conf /* = NULL */) {
	startBuild(conf);
	for (auto& v : (*data)) add(v);
	endBuild();
}

void HuffmanModel::startBuild(const Config *conf) {
	mbdata.cnt.clear();
	mbdata.max_symbol_size = 127;
	if (conf != NULL) mbdata.max_symbol_size = conf->getInt("HUFFDT_MAX_SYM", 127);
	mbdata.n = 0;
	mbdata.cnt.clear();
}

void HuffmanModel::add(uint32_t val) {
	++mbdata.cnt[val];
	++mbdata.n;
}

void HuffmanModel::endBuild() {
	buildModel_cnt(mbdata.n, mbdata.cnt, mbdata.max_symbol_size);
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
	//coder::DeltaCoder dc;
	if (freq.size() > 0) {
		auto it = freqset.find(val);
		if (it != freqset.end()) {
			auto cd = hc.encode(it->second);
			out->puts(cd);
		} else {
			out->puts(hc.encode(0));
			auto cd = coder::DeltaCoder::encode(val + 1);
			out->puts(cd);
		}
	}else {
		auto cd = coder::DeltaCoder::encode(val + 1);
		out->puts(cd);
	}
}

uint32_t HuffmanModel::decode(IWBitStream * is) const {
	//coder::DeltaCoder dc;
	unsigned int val = 0;
	if (freq.size() > 0) {
		auto a = tc.decode2(is->peek());
		is->skipw(a.second);
		if (a.first > 0)
			val = freq[a.first - 1];
		else {
			a = coder::DeltaCoder::decode2(is->peek());
			val = a.first - 1;
			is->skipw(a.second);
		}
	} else {
		auto a = coder::DeltaCoder::decode2(is->peek());
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

void HuffmanModel::inspect(const std::string& cmd, std::ostream& out) const {
	out << "huffman model" << '\n';
	out << "size = " << freq.size() << '\n';
	out << "# symbol  value  codelen" << "\n";
	out << 0 << " " << '_' << " " << hc.codelen(0) << '\n';
	for (unsigned int i = 0; i < freq.size(); ++i) {
		out << i+1 << " " << freq[i] << " " << hc.codelen(i+1) << '\n';
	}
}



//------------------------------------------------------------------------------
}
