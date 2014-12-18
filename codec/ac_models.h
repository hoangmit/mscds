#pragma once


#include <iostream>

#include "codec/arithmetic_code.hpp"
#include <cassert>
#include <algorithm>
#include <vector>

namespace coder {

class StaticAC_Model {
public:
	StaticAC_Model(unsigned int _alph_cnt = 0): alph_cnt(_alph_cnt), cum_freq(_alph_cnt + 1), total(0) {}

	void add(unsigned int val) {
		if (val >= alph_cnt) {
			alph_cnt = val + 1;
			cum_freq.resize(alph_cnt + 1);
		}
		cum_freq[val] += 1;
		total += 1;
	}

	void init_model(int size = -1) {
		size_t sum = 0;
		for (unsigned int i = 0; i < cum_freq.size(); ++i) {
			size_t nx = sum + cum_freq[i];
			cum_freq[i] = sum;
			sum = nx;
		}
		assert(cum_freq[alph_cnt] == total);
	}

	void start() {}

	unsigned int total_range() {
		return total;
	}

	std::pair<unsigned int, unsigned int> map_range(unsigned int val) {
		assert(val < alph_cnt);
		return std::make_pair(cum_freq[val], cum_freq[val+1]);
	}

	unsigned int unmap(unsigned int dc) {
		unsigned int idx = std::upper_bound(cum_freq.begin(), cum_freq.end(), dc) - cum_freq.begin();
		assert(idx > 0 && idx <= alph_cnt);
		return idx - 1;
	}

	void next() { }
private:
	unsigned int alph_cnt;
	std::vector<size_t> cum_freq;
	size_t total;
};


class SemiStaticAC_Model {
public:
	SemiStaticAC_Model(unsigned int _alph_cnt = 0): alph_cnt(_alph_cnt), cum_freq(_alph_cnt + 1), total(0) {}

	void add(unsigned int val) {
		if (val >= alph_cnt) {
			alph_cnt = val + 1;
			cum_freq.resize(alph_cnt + 1, 0);
		}
		cum_freq[val] += 1;
		total += 1;
	}

	void init_model(int size = -1) {
		size_t sum = 0;
		for (unsigned int i = 0; i < cum_freq.size(); ++i) {
			size_t nx = sum + cum_freq[i];
			cum_freq[i] = sum;
			sum = nx;
		}
		assert(cum_freq[alph_cnt] == total);
		update.resize(alph_cnt + 1);
		backup.resize(alph_cnt + 1);
		std::copy(cum_freq.begin(), cum_freq.end(), backup.begin());
		cycles = 0;
	}

	void start() {
		std::copy(backup.begin(), backup.end(), cum_freq.begin());
		std::fill(update.begin(), update.end(), 0);
		total = cum_freq[alph_cnt];
		cycles = 0;
	}

	unsigned int total_range() {
		return total;
	}

	std::pair<unsigned int, unsigned int> map_range(unsigned int val) {
		assert(val < alph_cnt);
		last = val;
		return std::make_pair(cum_freq[val], cum_freq[val+1]);
	}

	unsigned int unmap(unsigned int dc) {
		unsigned int idx = std::upper_bound(cum_freq.begin(), cum_freq.end(), dc) - cum_freq.begin();
		assert(idx > 0 && idx <= alph_cnt);
		last = idx - 1;
		return last;
	}

	void next() {
		if (cycles >= 5 * alph_cnt)
			update_model();
		update[last] += 1;
		cycles += 1;
	}

private:
	void update_model() {
		size_t sum = 0;
		assert(cum_freq.size() == update.size());
		for (unsigned int i = 0; i < update.size(); ++i) {
			size_t nx = sum + update[i];
			update[i] = sum;
			sum = nx;
		}
		for (unsigned int i = 0; i < cum_freq.size(); ++i) {
			cum_freq[i] -= update[i];
		}
		assert(total >= sum);
		total -= sum;
		assert(cum_freq[alph_cnt] == total);
		std::fill(update.begin(), update.end(), 0);
		cycles = 0;
	}
private:
	unsigned int alph_cnt;
	std::vector<size_t> cum_freq, update, backup;
	size_t total;
	unsigned int cycles;
	unsigned int last;
};


class AdaptiveAC_Model {
public:
	AdaptiveAC_Model(): alph_cnt(0) {}

	void add(unsigned int val) {
		if (val >= alph_cnt)
			alph_cnt = val + 1;
	}

	void init_model(int size = -1) {
		if (size > 0) { alph_cnt = size; }
		cum_freq.resize(alph_cnt + 1);
		update.resize(alph_cnt + 1);
		start();
	}

	void start() {
		for (unsigned int i = 0; i < cum_freq.size(); ++i)
			cum_freq[i] = i;
		total = alph_cnt;
		std::fill(update.begin(), update.end(), 0);
		cycles = 0;
	}

	unsigned int total_range() {
		return total;
	}

	std::pair<unsigned int, unsigned int> map_range(unsigned int val) {
		assert(val < alph_cnt);
		last = val;
		return std::make_pair(cum_freq[val], cum_freq[val+1]);
	}

	unsigned int unmap(unsigned int dc) {
		unsigned int idx = std::upper_bound(cum_freq.begin(), cum_freq.end(), dc) - cum_freq.begin();
		assert(idx > 0 && idx <= alph_cnt);
		last = idx - 1;
		return last;
	}

	void next() {
		if (cycles >= 8 * alph_cnt)
			update_model();
		update[last] += 1;
		cycles += 1;
	}
private:
	void update_model() {
		size_t sum = 0;
		assert(cum_freq.size() == update.size());
		const unsigned int dx = (2*alph_cnt + 1);
		for (unsigned int i = 0; i < update.size(); ++i) {
			unsigned int val =  (update[i] != 0) ? update[i]*dx : 1;
			size_t nx = sum + val;
			update[i] = sum;
			sum = nx;
		}
		std::swap(cum_freq, update);
		std::fill(update.begin(), update.end(), 0);
		total = sum;
		cycles = 0;
	}
private:
	unsigned int alph_cnt;
	std::vector<size_t> cum_freq, update;
	size_t total;
	unsigned int cycles;
	unsigned int last;
};

//-------------------------------------------------------

struct AC_OutStream {
	AC_OutStream(): buf(), outbuf(&buf), enc(&outbuf) {}

	typedef coder::OutBitStream::ChrTp ChrTp;
	std::vector<ChrTp> buf;
	coder::OutBitStream outbuf;
	coder::AC32_EncState enc;

	size_t bitlength() const {
		return outbuf.bitlen;
	}

	void close() {
		enc.close();
	}
};

struct AC_InpStream {
	AC_InpStream(): buf(), inbuf(&buf), dec(&inbuf)  {}

	void recv(AC_OutStream& other) {
		buf.swap(other.buf);
		inbuf.init(&buf, other.bitlength());
		dec.init(&inbuf);
	}

	typedef coder::InBitStream::ChrTp ChrTp;
	std::vector<ChrTp> buf;
	coder::InBitStream inbuf;
	coder::AC32_DecState dec;

	size_t bitlength() const {
		return inbuf.bitlen;
	}

	void close() {
		dec.close();
	}
};

template<class Model>
class EncBinder {
public:
	EncBinder(): inp(NULL), model(NULL) {}

	void bind(AC_OutStream * _out, Model * _model) {
		this->inp = _out;
		this->model = _model;
		_model->start();
	}

	void encode(unsigned int val) {
		unsigned int total = model->total_range();
		std::pair<unsigned int, unsigned int> p = model->map_range(val);
		inp->enc.update(p.first, p.second, total);
		model->next();
	}

	void close() {
		inp->close();
	}
private:
	Model * model;
	AC_OutStream * inp;
};

template<class Model>
class DecBinder {
public:
	DecBinder(): out(NULL), model(NULL) {}

	void bind(AC_InpStream * _inp, Model * _model) {
		this->out = _inp;
		this->model = _model;
		_model->start();
	}

	unsigned int decode() {
		unsigned int total = model->total_range();
		unsigned int dc = out->dec.decode_count(total);
		unsigned int val = model->unmap(dc);
		std::pair<unsigned int, unsigned int> p = model->map_range(val);
		out->dec.update(p.first, p.second, total);
		model->next();
		return val;
	}

	void close() {
		out->close();
	}
private:
	Model * model;
	AC_InpStream * out;
};

template<class Model, class ChrTp = uint8_t>
size_t ac_enc_size(const std::vector<ChrTp>& bv) {
	// create model
	Model mod;
	for (ChrTp c: bv)
		mod.add(c);
	mod.init_model();

	// encoding
	AC_OutStream outx;
	EncBinder<Model> ex;
	ex.bind(&outx, &mod);
	for (ChrTp c: bv)
		ex.encode(c);
	ex.close();
	size_t ret = (outx.bitlength() + 7) / 8;
	return ret;
	/*	
	// decoding
	AC_InpStream inx;
	inx.recv(outx);
	DecBinder<Model> dex;
	dex.bind(&inx, &mod);
	for (unsigned int i = 0; i < bv.size(); ++i) {
		//std::cout << i << std::endl;
		auto c = bv[i];
		uint8_t d = dex.decode();
		assert(c == d);
	}
	dex.close();
	*/
}

}//namespace
