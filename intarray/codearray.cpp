#include "codearray.h"

namespace mscds {
	DeltaCodeArrBuilder::DeltaCodeArrBuilder() {
		clear();
	}

	void DeltaCodeArrBuilder::init(unsigned int rate) {
		clear();
		sample_rate = rate;
	}

	void DeltaCodeArrBuilder::clear() {
		i = 0;
		sample_rate = 32;
		enc.clear();
		ptrbd.clear();
	}

	void DeltaCodeArrBuilder::add(uint64_t val) {
		if (i % sample_rate == 0)
			ptrbd.add_inc(enc.length());
		enc.puts(dc.encode(val));
		++i;
	}

	void DeltaCodeArrBuilder::build(OArchive &ar) {
		DeltaCodeArr tmp;
		tmp.save(ar);
	}

	void DeltaCodeArrBuilder::build(DeltaCodeArr *out) {
		enc.close();
		out->clear();
		out->sample_rate = sample_rate;
		out->len = i;
		out->enc = BitArray::create(enc.data_ptr(), enc.length());
		ptrbd.build(&(out->ptr));
	}

	DeltaCodeArr::Enumerator DeltaCodeArr::getEnum(uint64_t pos) const {
		uint64_t p = ptr.prefixsum(pos / sample_rate + 1);
		const unsigned int r = pos % sample_rate;
		Enumerator e;
		e.is.init(enc.data_ptr(), enc.length(), p);
		for (unsigned int i = 0; i < r; ++i)
			e.next();
		return e;
	}

	bool DeltaCodeArr::Enumerator::hasNext() const {
		return !(is.empty());
	}

	uint64_t DeltaCodeArr::Enumerator::next() {
		c = dc.decode2(is.peek());
		is.skipw(c.second);
		return c.first;
	}

	uint64_t DeltaCodeArr::lookup(uint64_t pos) const {
		return getEnum(pos).next();
	}

	void DeltaCodeArr::save(OArchive &ar) const {
		ar.startclass("delta_code_array", 1);
		ar.var("length").save(len);
		ar.var("sample_rate").save(sample_rate);
		ptr.save(ar.var("ptr"));
		enc.save(ar.var("enc"));
		ar.endclass();
	}

	void DeltaCodeArr::clear() {
		len = 0;
		sample_rate = 0;
		ptr.clear();
		enc.clear();
	}

	void DeltaCodeArr::load(IArchive &ar) {
		ar.loadclass("delta_code_array");
		ar.var("length").load(len);
		ar.var("sample_rate").load(sample_rate);
		ptr.load(ar.var("ptr"));
		enc.load(ar.var("enc"));
		ar.endclass();
	}


//---------------------------------------------------------------------

	DiffDeltaArrBuilder::DiffDeltaArrBuilder() {
		clear();
	}

	void DiffDeltaArrBuilder::init(unsigned int rate) {
		clear();
		sample_rate = rate;
	}

	void DiffDeltaArrBuilder::clear() {
		i = 0;
		sample_rate = 32;
		enc.clear();
		ptrbd.clear();
		lastval = 0;
	}

	void DiffDeltaArrBuilder::add(uint64_t val) {
		if (i % sample_rate == 0) {
			ptrbd.add_inc(enc.length());
			enc.puts(dc.encode(val));
		}
		else {
			enc.puts(dc.encode(1+coder::absmap((int64_t)val - lastval)));
		}
		lastval = val;
		++i;
	}

	void DiffDeltaArrBuilder::build(OArchive &ar) {
		DiffDeltaArr tmp;
		tmp.save(ar);
	}

	void DiffDeltaArrBuilder::build(DiffDeltaArr *out) {
		enc.close();
		out->clear();
		out->sample_rate = sample_rate;
		out->len = i;
		out->enc = BitArray::create(enc.data_ptr(), enc.length());
		ptrbd.build(&(out->ptr));
	}

	DiffDeltaArr::Enumerator DiffDeltaArr::getEnum(uint64_t pos) const {
		uint64_t p = ptr.prefixsum(pos / sample_rate + 1);
		const unsigned int r = pos % sample_rate;
		Enumerator it;
		it.is.init(enc.data_ptr(), enc.length(), p);
		coder::CodePr c = it.dc.decode2(it.is.peek());
		it.val = c.first;
		it.is.skipw(c.second);
		for (unsigned int i = 0; i < r; ++i) it.next();
		return it;
	}

	bool DiffDeltaArr::Enumerator::hasNext() const {
		return !is.empty();
	}

	uint64_t DiffDeltaArr::Enumerator::next() {
		coder::CodePr c = dc.decode2(is.peek());
		uint64_t oval = val;
		if (hasNext()) {
			val += coder::absunmap(c.first) - 1;
			is.skipw(c.second);
		}
		return oval;
	}

	uint64_t DiffDeltaArr::lookup(uint64_t pos) const {
		return (getEnum(pos)).next();
	}

	void DiffDeltaArr::save(OArchive &ar) const {
		ar.startclass("delta_code_array", 1);
		ar.var("length").save(len);
		ar.var("sample_rate").save(sample_rate);
		ptr.save(ar.var("ptr"));
		enc.save(ar.var("enc"));
		ar.endclass();
	}

	void DiffDeltaArr::clear() {
		len = 0;
		sample_rate = 0;
		ptr.clear();
		enc.clear();
	}

	void DiffDeltaArr::load(IArchive &ar) {
		ar.loadclass("delta_code_array");
		ar.var("length").load(len);
		ar.var("sample_rate").load(sample_rate);
		ptr.load(ar.var("ptr"));
		enc.load(ar.var("enc"));
		ar.endclass();
	}


}
