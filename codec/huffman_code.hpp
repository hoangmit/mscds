#pragma once


#include "codec/coder.h"
#include "codec/deltacoder.h"

#include <vector>
#include <iostream>
#include <queue>
#include <utility>
#include <cassert>
#include <algorithm>
#include <functional>

namespace coder {

/*
template<class T1, class T2, class Pred = std::less<T> >
struct cmp_pair_second {
	bool operator()(const std::pair<T1,T2>&left, const std::pair<T1,T2>&right) const {
		Pred p;
		return p(left.second, right.second);
	}
};
*/

unsigned char reverse8b(unsigned char b) {
	return (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

uint32_t reverse32b(uint32_t v) {
	uint32_t c;
	unsigned char * p = (unsigned char *) &v;
	unsigned char * q = (unsigned char *) &c;
	q[3] = reverse8b(p[0]);
	q[2] = reverse8b(p[1]);
	q[1] = reverse8b(p[2]);
	q[0] = reverse8b(p[3]);
	return c;
}

template<typename WeightTp, typename WeightSum>
WeightSum huffman(const std::vector<WeightTp>& W, std::vector<std::pair<unsigned int, unsigned int> >& output_code) {
	if (W.empty()) {
		output_code.clear();
		return WeightSum();
	}
	unsigned int n = W.size();
	typedef std::pair<WeightTp, unsigned int> PP;
	std::priority_queue<PP, std::vector<PP>, std::greater<PP> > pq;
	unsigned int i = 0;
	for (typename std::vector<WeightTp>::const_iterator it = W.begin(); it != W.end(); ++it)
		pq.push(make_pair(*it, i++));
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
		pq.push(make_pair(t1.first + t2.first, newid));
	}
	assert(pq.empty());
	// compute code length
	std::vector<unsigned int> L(parent.size());
	L[L.size() - 1] = 0;
	WeightSum total = 0;
	for (int i = (int)L.size()-2; i >= 0; i--) {
		L[i] = 1 + L[parent[i]];
		if ((unsigned int) i <  n) total += ((WeightSum)W[i]) * L[i];
	}
	output_code.resize(W.size());
	std::vector<unsigned int> code = canonical_code(W.size(), L);
	for (unsigned int i = 0; i < W.size(); ++i) {
		output_code[i].first = code[i];
		output_code[i].second = L[i];
	}
	return total;
}

// input : number of symbol,  vector of code len for each symbol
// output : vector of code word
std::vector<unsigned int> canonical_code(size_t nsym, const std::vector<unsigned int>& L) {
	assert(L.size() >= nsym);
	// first --> code length, second --> symbol
	typedef std::pair<unsigned int, unsigned int> pairuu;
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
	std::vector<unsigned int> ret;
	ret.reserve(nsym);
	for (unsigned int i = 0; i < nsym; i++) {
		ret.push_back(reverse32b(CodeWord[i].second << (32 - L[i])));
	}
	return ret;
}



void buildtree(const std::vector<std::pair<unsigned int, unsigned int> >& code,
		std::vector<std::pair<int, int> >& out) {
	int i;
	out.resize(1);
	out.reserve(code.size() - 1);
	out[0].first = 0;
	out[0].second = 0;
	
	for (int i = 1; i <= (int)code.size(); i++) {
		unsigned int val = code[i-1].first;
		unsigned int node = 0;
		for (unsigned int j = 0; j < code[i-1].second - 1; ++j) {
			bool b = (val & 1) > 0;
			if (!b) {
				if (out[node].first > 0)
					node = out[node].first;
				else {
					out.push_back(std::make_pair(-i, -i));
					out[node].first = out.size() - 1;
					node = out.size() - 1;
				}
			} else {
				if (out[node].second > 0) 
					node = out[node].second;
				else {
					out.push_back(std::make_pair(-i, -i));
					out[node].second = out.size() - 1;
					node = out.size() - 1;
				}
			}
			val >>= 1;
		}
		bool b = (val & 1) > 0;
		if (!b) {
			if (out[node].first > 0) throw std::runtime_error("invalid prefix code");
			out[node].first = -i;
		} else {
			if (out[node].second > 0) throw std::runtime_error("invalid prefix code");
			out[node].second = -i;
		}
	}
}


//---------------------------------------------------------------------
struct adp_huffman_enc {
	adp_huffman_enc(): alp_size(0) {}
	adp_huffman_enc(size_t _alp_size): alp_size(_alp_size) {reset();}

	size_t alp_size;
	unsigned int i, updcyc;
	
	std::vector<unsigned int> symcnt;
	std::vector<std::pair<unsigned int, unsigned int> > code;

	void init(size_t _alp_size) {
		alp_size = _alp_size;
		reset();
	}

	void reset() {
		assert(alp_size > 0);
		i = 0;
		updcyc = alp_size;
		symcnt.resize(alp_size, 1);
		huffman<unsigned int, unsigned int>(symcnt, code);
	}

	bool update(unsigned int n) {
		assert(n < alp_size);
		symcnt[n]++;
		i++;
		if (i >= updcyc) {
			i = 0;
			if (updcyc < 128 * alp_size) updcyc *= 2;
			huffman<unsigned int, unsigned int>(symcnt, code);
			return true;
		}else return false;
	}

	CodePr encode_s(NumTp n) {
		assert(n < alp_size);
		return CodePr(code[n].first, code[n].second);
	}

	CodePr encode(NumTp n) {
		CodePr ret = encode_s(n);
		update(n);
		return ret;
	}
};


struct adp_huffman_dec {
	adp_huffman_dec(): alp_size(0) {}
	adp_huffman_dec(size_t _alp_size): alp_size(_alp_size) {reset();}

	size_t alp_size;
	unsigned int i, updcyc;
	
	std::vector<unsigned int> symcnt;
	std::vector<std::pair<int, int> > ctree;
	//std::vector<std::pair<unsigned int, unsigned int> > code;

	void init(size_t _alp_size) {
		alp_size = _alp_size;
		reset();
	}

	void reset() {
		i = 0;
		updcyc = alp_size;
		symcnt.resize(alp_size, 1);
		std::vector<std::pair<unsigned int, unsigned int> > code;
		huffman<unsigned int, unsigned int>(symcnt, code);
		buildtree(code, ctree);
	}

	bool update(unsigned int n) {
		assert(n < alp_size);
		symcnt[n]++;
		i++;
		if (i >= updcyc) {
			i = 0;
			if (updcyc < 128 * alp_size) updcyc *= 2;
			std::vector<std::pair<unsigned int, unsigned int> > code;
			huffman<unsigned int, unsigned int>(symcnt, code);
			buildtree(code, ctree);
			return true;
		}else return false;
	}

	CodePr decode_s(NumTp n) {
		int node = 0;
		uint16_t len = 0;
		do {
			len++;
			bool b = (n & 1) > 0;
			if (!b) node = ctree[node].first;
			else node = ctree[node].second;
			n >>= 1;
		} while (node > 0);
		if (node == 0) throw std::runtime_error("wrong code tree");
		return CodePr(-1 - node, len);
	}

	CodePr decode(NumTp n) {
		CodePr ret = decode_s(n);
		update(ret.first);
		return ret;
	}
};

//----------------------------------------------------------

struct adp_huffman_ext {
	adp_huffman_ext() {}
	adp_huffman_ext(size_t _alp_size): base(_alp_size + 1), alp_size(_alp_size) {}

	DeltaCoder dtc;
	size_t alp_size;

	adp_huffman_enc base;

	void init(size_t _alp_size) {
		base.init(_alp_size+1);
		alp_size = _alp_size;
	}

	void reset() {
		base.reset();
	}
	void update(unsigned int n){
		if (n < alp_size) base.update(n);
		else base.update(alp_size);
	}
	CodePr encode_s(NumTp n) {
		if (n < alp_size) return base.encode(n);
		else return concatc(base.encode(alp_size), dtc.encode(n - alp_size + 1));
	}

	CodePr encode(NumTp n) {
		CodePr ret = encode_s(n);
		update(n);
		return ret;
	}
};

}