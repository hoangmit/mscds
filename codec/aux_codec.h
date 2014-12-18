#pragma once

#include <vector>
#include <cassert>

template<typename ValTp, class PosEnc, class EntropyEnc>
class RunLenEnc {
public:
	RunLen(PosEnc* _len, EntropyEnc * _val): cur_len(0), len_enc(_len), val_enc(_val) {}

	void add(ValTp val) {
		if (val != cur_val) {
			if (cur_len > 0) {
				len_enc->add(cur_len);
				val_enc->add(cur_val);
			}
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

template<typename ValTp>
class MTFBdList {
public:
	typedef int IdxTp;
	MTFList(unsigned int msize) {
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
private:
	std::vector<ValTp> lst;
	unsigned int max_size;
};

template<typename ValTp, class EntropyEnc1, class EntropyEnc2 = EntropyEnc1>
class MTFEnc {
public:
	MTFEnc(unsigned int _max_size, EntropyEnc1* ctl, EntropyEnc2* val): max_size(_max_size), control(ctl), value(val) {
		assert(max_size > 0);
		lst.reserve(max_size);
	}

	void add(ValTp val) {
		int idx = lst.findadd(val);
		// save to streams
		if (idx < 0) {
			control->add(max_size);
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
	MTFDec(unsigned int _max_size, EntropyDec1 * ctl, EntropyEnc2 * val): max_size(_max_size), control(ctl), value(val) {
		assert(max_size > 0);
		lst.reserve(max_size);
	}

	ValTp get() {
		unsigned int idx = control->get();
		ValTp v;
		if (idx == max_size) {
			v = value->get();
			lst.replace(idx, v);
		} else
			v = lst.get(idx);
		return v;
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



