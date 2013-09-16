#include "huffman_code.h"
#include <algorithm>

namespace coder {

inline unsigned char reverse8b(unsigned char b) {
	return (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

inline uint32_t reverse32b(uint32_t v) {
	uint32_t c;
	unsigned char * p = (unsigned char *) &v;
	unsigned char * q = (unsigned char *) &c;
	q[3] = reverse8b(p[0]);
	q[2] = reverse8b(p[1]);
	q[1] = reverse8b(p[2]);
	q[0] = reverse8b(p[3]);
	return c;
}

inline uint64_t reverse64b(uint64_t v) {
	uint64_t c;
	uint32_t * p = (uint32_t *) &v;
	uint32_t * q = (uint32_t *) &c;
	q[1] = reverse32b(p[0]);
	q[0] = reverse32b(p[1]);
	return c;
}

std::vector<uint32_t> HuffmanCode::canonical_code(size_t nsym, const std::vector<uint16_t>& L) {
	assert(L.size() >= nsym);
	// first --> code length, second --> symbol
	typedef std::pair<uint16_t, unsigned int> pairuu;
	std::vector<pairuu > CodeLen;
	CodeLen.reserve(nsym);
	for (unsigned int i = 0; i < nsym; i++) {
		CodeLen.push_back(pairuu(L[i], i));
	}
	// sort by code len
	sort(CodeLen.begin(), CodeLen.end());
	std::vector<std::pair<unsigned int, unsigned int> > CodeWord;
	// construct canonical code
	CodeWord.reserve(nsym);
	unsigned int code = 0;
	for (unsigned int i = 0; i < nsym - 1; i++) {
		CodeWord.push_back(pairuu(CodeLen[i].second, code));
		code = (code + 1) << ((CodeLen[i + 1].first) - (CodeLen[i].first));
	}
	CodeWord.push_back(pairuu(CodeLen[nsym - 1].second, code));
	sort(CodeWord.begin(), CodeWord.end());
	std::vector<uint32_t> ret;
	ret.reserve(nsym);
	for (unsigned int i = 0; i < nsym; i++) {
		ret.push_back(reverse32b(CodeWord[i].second << (32 - L[i])));
	}
	return ret;
}

HuffmanCode::WeightSum HuffmanCode::build(const std::vector<HuffmanCode::WeightTp>& W) {
	if (W.empty()) {
		output_code.clear();
		return WeightSum();
	}
	unsigned int n = W.size();
	typedef std::pair<WeightTp, unsigned int> PP;
	std::priority_queue<PP, std::vector<PP>, std::greater<PP> > pq;
	unsigned int i = 0;
	for (std::vector<WeightTp>::const_iterator it = W.begin(); it != W.end(); ++it)
		pq.push(std::make_pair(*it, i++));
	std::vector<unsigned int> parent(W.size());
	parent.reserve(W.size()*2 - 1);
	assert(!pq.empty());
	while (true) {
		PP t1 = pq.top();
		pq.pop();
		if (pq.empty()) break;
		PP t2 = pq.top();
		pq.pop();
		parent.push_back(0);
		unsigned int newid = parent.size() - 1;
		parent[t1.second] = newid;
		parent[t2.second] = newid;
		pq.push(std::make_pair(t1.first + t2.first, newid));
	}
	assert(pq.empty());
	// compute code length
	std::vector<uint16_t> L(parent.size());
	L[L.size() - 1] = 0;
	WeightSum total = 0;
	for (int i = (int)L.size()-2; i >= 0; i--) {
		L[i] = 1 + L[parent[i]];
		if ((unsigned int) i <  n) total += ((WeightSum)W[i]) * L[i];
	}
	loadCode(W.size(), L);
	return total;
}

void HuffmanCode::loadCode(size_t n, const std::vector<uint16_t>& L ) {
	output_code.resize(n);
	std::vector<unsigned int> code = canonical_code(n, L);
	for (unsigned int i = 0; i < n; ++i) {
		output_code[i].first = code[i];
		output_code[i].second = L[i];
	}
}



void HuffmanTree::build(const HuffmanCode& code) {
	//int i;
	tree.resize(1);
	tree.reserve(code.size() - 1);
	tree[0].first = 0;
	tree[0].second = 0;
	_size = code.size();

	for (int i = 1; i <= (int)code.size(); i++) {
		unsigned int val = code[i-1].first;
		unsigned int node = 0;
		for (unsigned int j = 0; j < code[i-1].second - 1; ++j) {
			bool b = (val & 1) > 0;
			if (!b) {
				if (tree[node].first > 0)
					node = tree[node].first;
				else {
					tree.push_back(std::make_pair(-i, -i));
					tree[node].first = tree.size() - 1;
					node = tree.size() - 1;
				}
			} else {
				if (tree[node].second > 0) 
					node = tree[node].second;
				else {
					tree.push_back(std::make_pair(-i, -i));
					tree[node].second = tree.size() - 1;
					node = tree.size() - 1;
				}
			}
			val >>= 1;
		}
		bool b = (val & 1) > 0;
		if (!b) {
			if (tree[node].first > 0) throw std::runtime_error("invalid prefix code");
			tree[node].first = -i;
		} else {
			if (tree[node].second > 0) throw std::runtime_error("invalid prefix code");
			tree[node].second = -i;
		}
	}
}

void HuffmanTree::build(const std::vector<HuffmanCode::WeightTp>& W) {
	HuffmanCode code;
	code.build(W);
	build(code);
}

CodePr HuffmanTree::decode2(NumTp n) const {
	int node = 0;
	uint16_t len = 0;
	do {
		len++;
		bool b = (n & 1) > 0;
		if (!b) node = tree[node].first;
		else node = tree[node].second;
		n >>= 1;
	} while (node > 0);
	if (node == 0) throw std::runtime_error("wrong code tree");
	return CodePr(-1 - node, len);
}

void HuffmanByteDec::build(const std::vector<HuffmanCode::WeightTp>& W) {
	HuffmanCode code;
	code.build(W);
	build(code);
}

void HuffmanByteDec::build(const HuffmanCode& hcode) {
	std::vector<CodeInfo> pcode(hcode.size());
	for (unsigned int i = 0; i < hcode.size(); ++i)
		pcode[i] = CodeInfo(hcode[i], i);
	sort(pcode.begin(), pcode.end());
	_size = hcode.size();
	int tb = 0;
	create(pcode, 0, pcode.size(), tb);
}

void HuffmanByteDec::create(std::vector<CodeInfo>& xc, unsigned int st, unsigned int ed, int & table_id) {
	unsigned int maxlen = 0;
	for (unsigned int k = st; k < ed; ++k)
		maxlen = std::max<unsigned int>(maxlen, xc[k].len);
	unsigned int cutlen = std::min(maxlen, 8u);
	unsigned int stpos = code.size();
	tbl_info.push_back(std::make_pair(stpos, cutlen));
	for (unsigned int k = 0; k < (1u << cutlen); ++k) {
		code.push_back(0);
	}
	unsigned int i = st;
	while (i < ed) {
		assert(xc[i].len > 0);
		if (xc[i].len > cutlen) {
			auto mask = ((1u << cutlen) - 1);
			auto v = xc[i].code & mask;
			unsigned int j = i;
			while (j < ed && xc[j].len > cutlen && ((xc[j].code & mask) == v)) {
				xc[j].len -= cutlen;
				xc[j].code >>= cutlen;
				j++;
			}
			assert(j - i > 1);
			table_id += 1;
			code[stpos + v] = -table_id;
			create(xc, i, j, table_id);
			i = j;
			continue;
		}else {
			const CodeInfo& cur = xc[i];
			assert(cur.len > 0);
			unsigned int t = ((cur.len - 1) << 12) | cur.sym;
			auto s = cutlen - cur.len;
			for (unsigned int j = 0; j < (1 << s); ++j) {
				auto p = (j << cur.len) | cur.code;
				code[stpos + p] = t;
			}
			xc[i].len = 0;
			xc[i].code = 0;
		}
		++i;
	}
}


coder::CodePr HuffmanByteDec::decode2(NumTp n) const {
	unsigned int tb = 0, tblen = tbl_info[0].second, tb_start = 0;
	unsigned int prefix = 0; 
	while (prefix < 64) {
		NumTp x = n & ((1ULL << tblen) - 1);
		n = n >> tblen;
		int16_t val = code[tb_start + x];
		if (val >= 0) {
			return CodePr(val & 0x0FFF, prefix + (val >> 12) + 1);
		} else {
			tb = -val;
			prefix += tblen;
			tb_start = tbl_info[tb].first;
			tblen = tbl_info[tb].second;
		}
	}
	assert(false);//cannot reach here
}

HuffmanByteDec::HuffmanByteDec(): _size(0) {}

void HuffmanByteDec::clear() {
	_size = 0;
	code.clear();
	tbl_info.clear();
}

bool HuffmanByteDec::CodeInfo::operator<(const CodeInfo& b) const {
	return reverse64b(code) < reverse64b(b.code);
}

}//namespace
