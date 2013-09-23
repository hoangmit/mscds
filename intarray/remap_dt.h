#pragma once

#include <unordered_map>
#include <map>

#include "bitarray/bitstream.h"
#include "codec/deltacoder.h"
#include "blkarray.hpp"

#include "diffarray.hpp"
#include "utils/utils.h"

#include "transform_utils.hpp"
#include "contextfree_models.hpp"

namespace mscds {


class RemapTransform {
public:
	typedef uint32_t InputTp;
	typedef uint32_t OutputTp;

	void buildModel(const std::vector<InputTp> * data, unsigned int optional = 0);
	void saveModel(OBitStream * out) const;
	void loadModel(IWBitStream & is, bool decode_only = false);

	void clear();

	OutputTp map(InputTp val) const;
	InputTp unmap(OutputTp) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	std::unordered_map<uint32_t, uint32_t> remap, rev;
	void buildRev();
};

typedef BindModel<RemapTransform, DeltaModel> RemapDtModel;

}

REGISTER_PARSE_TYPE(mscds::RemapDtModel);


namespace mscds {

typedef CodeModelArray<RemapDtModel> RemapDtArray;
typedef CodeModelBuilder<RemapDtModel> RemapDtArrayBuilder;

typedef DiffArray<RemapDtArray> RemapDiffDtArray;
typedef DiffArrayBuilder<RemapDtArray> RemapDiffArrBuilder;

}
