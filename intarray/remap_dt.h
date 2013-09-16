#pragma once

#include <unordered_map>
#include <map>

#include "bitarray/bitstream.h"
#include "codec/deltacoder.h"
#include "blkarray.hpp"

#include "diffarray.hpp"
#include "utils/utils.h"

namespace mscds {

	class RemapDtModel {
	public:
		void buildModel(std::vector<uint32_t> * data, unsigned int optional = 0);
		void saveModel(OBitStream * out) const;
		void loadModel(IWBitStream & is, bool decode_only = false);

		void clear();

		void encode(uint32_t val, OBitStream * out) const;
		uint32_t decode(IWBitStream * is) const;
		void inspect(const std::string& cmd, std::ostream& out) const;
	private:
		std::unordered_map<uint32_t, uint32_t> remap, rev;
		void buildRev();
	};
}

REGISTER_PARSE_TYPE(mscds::RemapDtModel);


namespace mscds {

typedef CodeModelArray<RemapDtModel> RemapDtArray;
typedef CodeModelBuilder<RemapDtModel> RemapDtArrayBuilder;

typedef DiffArray<RemapDtArray> RemapDiffDtArray;
typedef DiffArrayBuilder<RemapDtArray> RemapDiffArrBuilder;

}
