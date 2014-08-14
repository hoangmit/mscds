
#include "generic_struct.h"

namespace mscds {

void StructIDList::addId(const std::string &name) {
	_lst.push_back(-1);
}

void StructIDList::checkId(const std::string &name) {
	//int v = _lst.front(); _lst.pop_front();
	int v = _lst[pfront];
	pfront++;
	if (v != -1) throw std::runtime_error("failed");
}

void StructIDList::add(unsigned int id) {
	_lst.push_back((int)id);
}

unsigned int StructIDList::get() {
	//int v = (int)_lst.front(); _lst.pop_front();
	int v = _lst[pfront];
	pfront++;
	assert(v > 0);
	return (unsigned int)v;
}

void StructIDList::save(mscds::OutArchive &ar) const {
	ar.startclass("block_struct_list", 1);
	uint32_t len = _lst.size();
	ar.var("len").save(len);
	ar.var("list");
	for (unsigned int i = 0; i < len; ++i) {
		int16_t v = _lst[i];
		ar.save_bin(&v, sizeof(v));
	}
	ar.endclass();
}

void StructIDList::load(mscds::InpArchive &ar) {
	int class_version = ar.loadclass("block_struct_list");
	uint32_t len = 0;
	ar.var("len").load(len);
	ar.var("list");
	for (unsigned int i = 0; i < len; ++i) {
		int16_t v = 0;
		ar.load_bin(&v, sizeof(v));
		_lst.push_back(v);
	}
	ar.endclass();
}

void StructIDList::clear() {
	_lst.clear();
	pfront = 0;
}

void StructIDList::reset() {
	pfront = 0;
}


}//namespace