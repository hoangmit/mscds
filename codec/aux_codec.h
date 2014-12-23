#pragma once

#include <vector>
#include <iostream>
#include <cassert>

namespace coder {

template<typename ValTp, class PosEnc, class EntropyEnc>
class RunLenEnc {
public:
	RunLenEnc(PosEnc* _len, EntropyEnc * _val):
		cur_len(0), len_enc(_len), val_enc(_val) {}

	void add(ValTp val) {
		if (val != cur_val) {
			if (cur_len > 0) {
				len_enc->add(cur_len);
				val_enc->add(cur_val);
			}
			cur_len = 1;
			cur_val = val;
		} else
			cur_len += 1;
	}

	void reset() {
		cur_len = 0;
	}

	void close() {
		if (cur_len > 0) {
			len_enc->add(cur_len);
			val_enc->add(cur_val);
			reset();
		}
	}
private:
	PosEnc * len_enc;
	EntropyEnc * val_enc;
	size_t cur_len;
	ValTp cur_val;
};

template<typename ValTp, class PosDec, class EntropyDec>
class RunLenDec {
public:
	RunLenDec(PosDec* _len, EntropyDec * _val):
		cur_len(0), len_dec(_len), val_dec(_val) {}

	ValTp get() {
		if (cur_len == 0) {
			cur_len = len_dec->get();
			cur_val = val_dec->get();
			assert(cur_len > 0);
		}
		cur_len -= 1;
		return cur_val;
	}

	bool hasNext() {
		return (cur_len > 0) || (len_dec->hasNext());
	}

	void reset() {
		cur_len = 0;
	}

	void close() {
	}
private:
	PosDec * len_dec;
	EntropyDec * val_dec;

	size_t cur_len;
	ValTp cur_val;
};

template<typename ValTp>
class MTFBdList {
public:
	typedef int IdxTp;
	MTFBdList(unsigned int msize) {
		max_size = msize;
		lst.reserve(max_size);
	}

	ValTp get(unsigned int idx) {
		assert(idx < lst.size());
		ValTp ret = lst[idx];
		replace(idx, ret);
		return ret;
	}

	IdxTp findadd(ValTp val) {
		assert(lst.size() <= max_size);
		IdxTp idx = -1;
		for (unsigned int i = 0; i < lst.size(); ++i)
			if (lst[i] == val) {
				idx = i;
				break;
			}
		replace(idx, val);
		return idx;
	}

	void replace(IdxTp idx, ValTp val) {
		// modify mtf list
		unsigned int rid;
		if (idx >= 0) rid = idx;
		else {
			if (lst.size() >= max_size) rid = lst.size() - 1;
			else { rid = lst.size(); lst.push_back(val); }
		}
		for (unsigned int i = rid; i > 0; i--) lst[i] = lst[i-1];
		lst[0] = val;
	}

	void clear() {
		lst.clear();
	}
	unsigned int get_max_size() const {
		return max_size;
	}
private:
	std::vector<ValTp> lst;
	unsigned int max_size;
};

template<typename ValTp, class EntropyEnc1, class EntropyEnc2 = EntropyEnc1>
class MTFEnc {
public:
	MTFEnc(unsigned int _max_size, EntropyEnc1* ctl, EntropyEnc2* val):
		lst(_max_size), control(ctl), value(val) {
	}

	void add(ValTp val) {
		int idx = lst.findadd(val);
		// save to streams
		if (idx < 0) {
			control->add(lst.get_max_size());
			value->add(val);
		} else {
			control->add(idx);
		}
	}


	void reset() {
		lst.clear();
	}

	void close() {
		reset();
	}
private:
	MTFBdList<ValTp> lst;

	EntropyEnc1 * control;
	EntropyEnc2 * value;
};

template<typename ValTp, class EntropyDec1, class EntropyDec2 = EntropyDec1>
class MTFDec {
public:
	MTFDec(unsigned int _max_size, EntropyDec1 * ctl, EntropyDec2 * val):
		lst(_max_size), control(ctl), value(val) {
	}

	ValTp get() {
		unsigned int idx = control->get();
		ValTp v;
		if (idx == lst.get_max_size()) {
			v = value->get();
			lst.replace(-1, v);
		} else
			v = lst.get(idx);
		return v;
	}

	bool hasNext() {
		return control->hasNext();
	}

	void reset() {
		lst.clear();
	}

	void close() {
		reset();
	}

private:
	MTFBdList<ValTp> lst;
	EntropyDec1 * control;
	EntropyDec2 * value;
};


template<typename ValTp>
struct DequeStream {
	ValTp get() {
		ValTp v = data.front();
		data.pop_front();
		//std::cout << "get " << v << std::endl;
		return v;
	}

	void add(ValTp v) {
		//std::cout << "add " << v << std::endl;
		data.push_back(v);
	}

	bool hasNext() {
		return !(data.empty());
	}

	void reset() {}

	void close() {}

	std::deque<ValTp> data;
};


}//namespace
