#pragma once

/**  \file

Experimental structures

*/

#include "bitarray/bitarray.h"
#include <stdint.h>
#include <vector>
#include "utils/param.h"

namespace mscds {

template<typename TransformModel, typename EncodeModel, typename DataTp=uint32_t>
struct BindModel {
public:
	void buildModel(const std::vector<DataTp> * data, const Config* conf = NULL);
	void saveModel(OBitStream * out) const;
	void loadModel(IWBitStream & is, bool decode_only = false);

	void clear();

	void encode(DataTp val, OBitStream * out) const;
	DataTp decode(IWBitStream * is) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	TransformModel tr;
	EncodeModel enc;
};


template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline void mscds::BindModel<TransformModel, EncodeModel, DataTp>::inspect(const std::string& cmd, std::ostream& out) const {
	tr.inspect(cmd, out);
	enc.inspect(cmd, out);
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline DataTp mscds::BindModel<TransformModel, EncodeModel, DataTp>::decode(IWBitStream * is) const {
	return tr.unmap(enc.decode(is));
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
void mscds::BindModel<TransformModel, EncodeModel, DataTp>::encode(DataTp val, OBitStream * out) const {
	enc.encode(tr.map(val), out);
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline void mscds::BindModel<TransformModel, EncodeModel, DataTp>::loadModel( IWBitStream & is, bool decode_only /*= false*/ ) {
	tr.loadModel(is, decode_only);
	enc.loadModel(is, decode_only);
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline void mscds::BindModel<TransformModel, EncodeModel, DataTp>::buildModel(const std::vector<DataTp> * data, const Config* conf){
	tr.buildModel(data, conf);
	enc.buildModel(data, conf);
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline void mscds::BindModel<TransformModel, EncodeModel, DataTp>::saveModel(OBitStream * out) const {
	tr.saveModel(out);
	enc.saveModel(out);
}

template<typename TransformModel, typename EncodeModel, typename DataTp/*=uint32_t*/>
inline void mscds::BindModel<TransformModel, EncodeModel, DataTp>::clear() {
	tr.clear();
	enc.clear();
}


}//namespace