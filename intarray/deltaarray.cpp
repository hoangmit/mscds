#include "deltaarray.h"

namespace mscds {
	DeltaCodeArrBuilder::DeltaCodeArrBuilder() {
		clear();
	}

	void DeltaCodeArrBuilder::init(const Config* conf) {
		clear();
		if (conf == NULL)
			sample_rate = 32;
		else
			sample_rate = conf->getInt("SAMPLE_RATE", 32);
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
		enc.puts(dc.encode(val + 1));
		++i;
	}

	void DeltaCodeArrBuilder::build(OutArchive &ar) {
		DeltaCodeArr tmp;
		tmp.save(ar);
	}

	void DeltaCodeArrBuilder::build(DeltaCodeArr *out) {
		enc.close();
		out->clear();
		out->sample_rate = sample_rate;
		out->len = i;
		enc.build(&out->enc);
		ptrbd.build(&(out->ptr));
	}

	void DeltaCodeArr::getEnum(uint64_t pos, Enum * e) const {
		uint64_t p = ptr.prefixsum(pos / sample_rate + 1);
		const unsigned int r = pos % sample_rate;
		e->is.init_array(enc, p);
		for (unsigned int i = 0; i < r; ++i)
			e->next();
	}

	bool DeltaCodeArr::Enum::hasNext() const {
		return !(is.empty());
	}

	uint64_t DeltaCodeArr::Enum::next() {
		c = dc.decode2(is.peek());
		is.skipw(c.second);
		return c.first - 1;
	}

	uint64_t DeltaCodeArr::lookup(uint64_t pos) const {
		Enum e;
		getEnum(pos, &e);
		return e.next();
	}

	void DeltaCodeArr::save(OutArchive &ar) const {
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

	void DeltaCodeArr::load(InpArchive &ar) {
		ar.loadclass("delta_code_array");
		ar.var("length").load(len);
		ar.var("sample_rate").load(sample_rate);
		ptr.load(ar.var("ptr"));
		enc.load(ar.var("enc"));
		ar.endclass();
	}

	void DeltaCodeArr::inspect(const std::string& cmd, std::ostream& out) const
	{

	}



}
