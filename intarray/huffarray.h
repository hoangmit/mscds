#pragma once

#include <utility>
#include <functional>
#include <vector>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include "codec/huffman_code.h"
#include "blkarray.hpp"
#include "utils/utils.h"
#include "diffarray.hpp"

namespace mscds {

class HuffmanModel {
public:
	void buildModel(const std::vector<uint32_t> * data, unsigned int max_symbol_size = 127);
	void saveModel(OBitStream * out) const;
	void loadModel(IWBitStream & is, bool decode_only = false);

	void clear();

	void encode(uint32_t val, OBitStream * out) const;
	uint32_t decode(IWBitStream * is) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	void buildModel2(const std::vector<uint32_t> * data);
	coder::HuffmanCode hc;
	coder::HuffmanByteDec tc;
	std::vector<uint32_t> freq;
	std::unordered_map<uint32_t, uint32_t> freqset; //unordered_
};
}//namespace

REGISTER_PARSE_TYPE(mscds::HuffmanModel);

namespace mscds {

typedef CodeModelArray<HuffmanModel> HuffmanArray;
typedef CodeModelBuilder<HuffmanModel> HuffmanArrBuilder;

typedef DiffArray<HuffmanArray> HuffDiffArray;
typedef DiffArrayBuilder<HuffmanArray> HuffDiffArrBuilder;

}//namespace
