#pragma once

#include "codec/deltacoder.h"

namespace mscds {
class DeltaModel {
public:
	void buildModel(const std::vector<uint32_t> * data, unsigned int opt = 0) { }
	void saveModel(OBitStream * out) const {}
	void loadModel(IWBitStream & is, bool decode_only = false) { }

	void clear() {}

	void encode(uint32_t val, OBitStream * out) const { out->puts(dc.encode(val+1)); }
	uint32_t decode(IWBitStream * is) const {
		auto a = dc.decode2(is->peek());
		is->skipw(a.second);
		return a.first - 1;
	}
	void inspect(const std::string& cmd, std::ostream& out) const { }
private:
	coder::DeltaCoder dc;
};


}//namespace
