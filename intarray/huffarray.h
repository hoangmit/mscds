#pragma once

#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <stdexcept>
#include <stdint.h>

#include "codec/huffman_code.h"
#include "blkarray.hpp"
#include "utils/utils.h"
#include "diffarray.hpp"

namespace mscds {

class HuffmanModel {
public:
	void buildModel2(std::vector<uint32_t> * data);
	void buildModel(std::vector<uint32_t> * data);
	void saveModel(OBitStream * out) const;
	void loadModel(IWBitStream & is, bool decode_only = false);

	void clear();

	void encode(uint32_t val, OBitStream * out) const;
	uint32_t decode(IWBitStream * is) const;
private:
	coder::HuffmanCode hc;
	coder::HuffmanTree tc;
	std::vector<uint32_t> freq;
	std::unordered_map<uint32_t, uint32_t> freqset; //unordered_

	void choosesyml_prob(std::vector<uint32_t> * data);
	void choosesyml_fixsize(std::vector<uint32_t> * data);
	void build_symmodel(std::vector<uint32_t> * data);
};
}

REGISTER_PARSE_TYPE(mscds::HuffmanModel);

namespace mscds {

typedef CodeModelArray<HuffmanModel> HuffmanArray;
typedef CodeModelBuilder<HuffmanModel> HuffmanArrBuilder;


typedef DiffArray<HuffmanArray> HuffDiffArray;
typedef DiffArrayBuilder<HuffmanArray> HuffDiffArrBuilder;

}//namespace
