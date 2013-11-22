#pragma once

#include "codec/deltacoder.h"
#include "utils/param.h"
#include "bitarray/bitstream.h"

namespace mscds {
class DeltaModel {
public:
	void buildModel(const std::vector<uint32_t> *, const Config* conf = NULL) { }
	void saveModel(OBitStream *) const {}
	void loadModel(IWBitStream &, bool decode_only = false) { }

	void clear() {}

	void encode(uint32_t val, OBitStream * out) const { out->puts(coder::DeltaCoder::encode(val + 1)); }
	uint32_t decode(IWBitStream * is) const {
		auto a = coder::DeltaCoder::decode2(is->peek());
		is->skipw(a.second);
		return a.first - 1;
	}
	void inspect(const std::string&, std::ostream&) const { }
private:
};


}//namespace
