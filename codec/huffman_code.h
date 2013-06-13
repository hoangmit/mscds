
#pragma once

#include <vector>
#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <queue>
#include <utility>
#include <functional>
#include "coder.h"

namespace coder {

class HuffmanCode {
public:
	typedef unsigned int WeightTp;
	typedef unsigned long long WeightSum;

	WeightSum build(const std::vector<WeightTp>& W);
	uint16_t codelen(unsigned int i) const { return output_code[i].second; }
	unsigned int codeword(unsigned int i) const { return output_code[i].first; }
	const CodePr& operator[](unsigned int i) const { return output_code[i]; }
	CodePr encode(unsigned int i) const { return output_code[i]; }
	unsigned int size() const { return output_code.size(); }

	// input : number of symbol,  vector of code len for each symbol
	// output : vector of code word
	static std::vector<uint32_t> canonical_code(size_t nsym, const std::vector<uint16_t>& L);
private:
	std::vector<CodePr> output_code;

	friend class HuffmanTree;
};


// http://codereview.stackexchange.com/questions/4479/faster-huffman-decoding
// http://www.gzip.org/algorithm.txt

class HuffmanTree {
public:
	HuffmanTree(): _size(0) {}
	void build(const HuffmanCode& code);
	void build(const std::vector<HuffmanCode::WeightTp>& W);
	CodePr decode2(NumTp n) const;
	unsigned int size() const { return _size; }
private:
	unsigned int _size;
	std::vector<std::pair<int, int> > tree;
};



}//namespace