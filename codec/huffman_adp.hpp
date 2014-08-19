#pragma once

/** 

Implement Adaptive huffman coding.
  
Concept of adaptive coding: http://en.wikipedia.org/wiki/Adaptive_coding

Written by Hoang.
*/

#include "huffman_code.h"
#include "deltacoder.h"

namespace coder {
/// adaptive Huffman codec
struct adp_huffman_enc {
	adp_huffman_enc(): alp_size(0) {}
	adp_huffman_enc(size_t _alp_size): alp_size(_alp_size) {reset();}

	size_t alp_size;
	unsigned int i, updcyc;
	
	std::vector<unsigned int> symcnt;
	HuffmanCode code;

	void init(size_t _alp_size) {
		alp_size = _alp_size;
		reset();
	}

	void reset() {
		assert(alp_size > 0);
		i = 0;
		updcyc = alp_size;
		symcnt.resize(alp_size, 1);
		code.build(symcnt);
	}

	bool update(unsigned int n) {
		assert(n < alp_size);
		symcnt[n]++;
		i++;
		if (i >= updcyc) {
			i = 0;
			if (updcyc < 128 * alp_size) updcyc *= 2;
			code.build(symcnt);
			return true;
		}else return false;
	}

	CodePr encode_s(NumTp n) {
		assert(n < alp_size);
		return code[n];
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
	HuffmanTree ctree;


	void init(size_t _alp_size) {
		alp_size = _alp_size;
		reset();
	}

	void reset() {
		i = 0;
		updcyc = alp_size;
		symcnt.resize(alp_size, 1);
		ctree.build(symcnt);
	}

	bool update(unsigned int n) {
		assert(n < alp_size);
		symcnt[n]++;
		i++;
		if (i >= updcyc) {
			i = 0;
			if (updcyc < 128 * alp_size) updcyc *= 2;
			ctree.build(symcnt);
			return true;
		}else return false;
	}

	CodePr decode_s(NumTp n) {
		return ctree.decode2(n);
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

}//namespace