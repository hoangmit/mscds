#pragma once

#include "codec/deltacoder.h"

namespace mscds {
class DeltaModel {
public:
	void buildModel(const std::vector<uint32_t> * data, const Config* conf = NULL) { }
	void saveModel(OBitStream * out) const {}
	void loadModel(IWBitStream & is, bool decode_only = false) { }

	void clear() {}

	void encode(uint32_t val, OBitStream * out) const { out->puts(coder::DeltaCoder::encode(val + 1)); }
	uint32_t decode(IWBitStream * is) const {
		auto a = coder::DeltaCoder::decode2(is->peek());
		is->skipw(a.second);
		return a.first - 1;
	}
	void inspect(const std::string& cmd, std::ostream& out) const { }
private:
};


}//namespace
