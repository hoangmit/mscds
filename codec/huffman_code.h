
#pragma once

/** 

Implement huffman coding.

Written by Hoang.

*/

#include <vector>
#include <stdint.h>
#include <cassert>
#include <algorithm>
#include <queue>
#include <utility>
#include <functional>
#include "coder.h"
#include "bitarray/bitop.h"

namespace coder {
/// Huffman codec
class HuffmanCode {
public:
	typedef unsigned int WeightTp;
	typedef unsigned long long WeightSum;

	template<typename Itr>
	static std::vector<WeightTp> comp_weight(unsigned int sz, Itr bg, Itr end) {
		std::vector<WeightTp> ret(sz, 0);
		for (Itr it = bg; it != end; ++it) ret[*it]++;
		return ret;
	}

	WeightSum build(const std::vector<WeightTp>& W);
	uint16_t codelen(unsigned int i) const { return output_code[i].second; }
	unsigned int codeword(unsigned int i) const { return output_code[i].first; }
	const CodePr& operator[](unsigned int i) const { return output_code[i]; }
	CodePr encode(unsigned int i) const { return output_code[i]; }
	unsigned int size() const { return (unsigned int) output_code.size(); }
	void clear() { output_code.clear(); }
	void loadCode(size_t n, const std::vector<uint16_t>& L);

	// input : number of symbol,  vector of code len for each symbol
	// output : vector of code word
	static std::vector<uint32_t> canonical_code(size_t nsym, const std::vector<uint16_t>& L);
private:
	std::vector<CodePr> output_code;

	friend class HuffmanTree;
};

class HuffmanTree {
public:
	HuffmanTree(): _size(0) {}
	void build(const HuffmanCode& code);
	void build(const std::vector<HuffmanCode::WeightTp>& W);
	CodePr decode2(NumTp n) const;
	unsigned int size() const { return _size; }
	void clear() { _size = 0; tree.clear(); }
private:
	unsigned int _size;
	std::vector<std::pair<int, int> > tree;
};

// http://codereview.stackexchange.com/questions/4479/faster-huffman-decoding
// http://www.gzip.org/algorithm.txt

class HuffmanByteDec {
public:
	HuffmanByteDec();
	void build(const HuffmanCode& code);
	void build(const std::vector<HuffmanCode::WeightTp>& W);
	CodePr decode2(NumTp n) const;
	unsigned int size() const;
	void clear();
private:
	unsigned int _size;

	//maximum 2^12 = 4096 symbols, max 256 per decode table
	//code: highest bit:
	//  0 --> terminal symbol, 3 bit for value of (length - 1), the rest 12-bits are the symbols
	//  1 --> link to other table ( = - table_id)
	std::vector<int16_t> code;
	// <start, prefix_len>
	typedef std::pair<uint16_t, uint16_t> Entry;
	std::vector< Entry > tbl_info;
	struct CodeInfo {
		uint64_t code;
		uint16_t len;
		unsigned int sym;
		CodeInfo() {}
		CodeInfo(CodePr c, unsigned int s): code(c.first), len(c.second), sym(s) {}
		bool operator< (const CodeInfo& b) const;
	};
	void create(std::vector<CodeInfo>& code, unsigned int st, unsigned int ed, int & table_id);
};

}//namespace