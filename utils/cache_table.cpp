#include "cache_table.h"

namespace utils {

CachePolicyInterface::OpResultTp LRU_Policy::check(const CachePolicyInterface::KeyTp &key) {
	auto it = map.find(key);
	if (it != map.end())
		return OpResultTp(it->second.first, FOUND_ENTRY);
	else
		return OpResultTp(0, NOT_FOUND);
}

CachePolicyInterface::OpResultTp LRU_Policy::access(const CachePolicyInterface::KeyTp &key) {
	auto it = map.find(key);
	if (it != map.end()) {
		auto retid = it->second.first;
		_access.splice(_access.end(), _access, it->second.second);
		return OpResultTp(retid, FOUND_ENTRY);
	} else
		return OpResultTp(0, NOT_FOUND);
}

CachePolicyInterface::OpResultTp LRU_Policy::update(const CachePolicyInterface::KeyTp &key) {
	auto it = map.find(key);
	if (it != map.end()) {
		auto retid = it->second.first;
		_access.splice(_access.end(), _access, it->second.second);
		return OpResultTp(retid, FOUND_ENTRY);
	} else {
		if (_capacity == 0) return OpResultTp(0, NOT_FOUND);
		if (map.size() == _capacity) {
			// take least frequent one
			auto rkey = _access.front();
			it = map.find(rkey);
			auto retid = it->second.first;
			_access.splice(_access.end(), _access, it->second.second);
			return OpResultTp(it->second.first, REPLACED_ENTRY);
		} else {
			auto newent = freelst.back();
			freelst.pop_back();
			_access.push_back(key);
			auto it = _access.end();
			--it;
			map[key] = std::make_pair(newent, it);
			return OpResultTp(newent, NEW_ENTRY);
		}
	}
}

CachePolicyInterface::OpResultTp LRU_Policy::remove(const CachePolicyInterface::KeyTp &key) {
	auto it = map.find(key);
	if (it != map.end()) {
		freelst.push_back(it->second.first);
		_access.erase(it->second.second);
		map.erase(it);
		return OpResultTp(0, FOUND_ENTRY);
	} else
		return OpResultTp(0, NOT_FOUND);
}

LRU_Policy::EntryInfoTp LRU_Policy::envict() {
	if (size() == 0) {
		return EntryInfoTp(0, 0);
	}
	else {
		auto rkey = _access.front();
		auto it = map.find(rkey);
		auto retid = it->second.first;
		_access.pop_front();
		freelst.push_back(retid);
		map.erase(it);
		return EntryInfoTp(rkey, retid);
	}
}

void LRU_Policy::clear() {
	map.clear();
	_access.clear();
	init(_capacity);
}

std::vector<LRU_Policy::EntryInfoTp> LRU_Policy::get_data() {
	std::vector<EntryInfoTp > out;
	out.reserve(map.size());
	for (auto it = map.begin(); it != map.end(); ++it)
		out.push_back(EntryInfoTp(it->first, it->second.first));
	return out;
}

void LRU_Policy::resize_capacity(size_t new_cap) {
	assert(new_cap > 0);
	if (new_cap >= _capacity) {
		for (unsigned int i = _capacity; i < new_cap; ++i)
			freelst.push_back(i);
		_capacity = new_cap;
	}
	else {
		clear();
		init(new_cap);
	}
}

void LRU_Policy::init(size_t capacity) {
	this->_capacity = capacity;
	for (unsigned int i = 0; i < _capacity; ++i)
		freelst.push_back(i);
}

}//namespace
