#pragma once

#include <list>
#include <utility>
#include <unordered_map>
#include <cassert>
#include <vector>
#include <cstddef>

namespace utils {

class CachePolicyInterface {
public:
	typedef unsigned int KeyTp;
	typedef unsigned int EntryIndexTp;
	
	enum EntryResultTp { NOT_FOUND, FOUND_ENTRY, REPLACED_ENTRY, NEW_ENTRY };
	
	struct OpResultTp {
		OpResultTp(): type(NOT_FOUND) {}
		OpResultTp(EntryIndexTp idx, EntryResultTp _type): index(idx), type(_type) {}
		EntryIndexTp index;
		EntryResultTp type;
	};

	/* check if table contains a key
	NOT_FOUND or FOUND_ENTRY
	*/
	virtual OpResultTp check(const KeyTp& key) = 0;
	
	/* find entry of a key; mark recent access
	NOT_FOUND or FOUND_ENTRY
	*/
	virtual OpResultTp access(const KeyTp& key) = 0;
	
	/* find existing key, find a place to update,
	FOUND_ENTRY or REPLACED_ENTRY/NEW_ENTRY
	*/
	virtual OpResultTp update(const KeyTp& key) = 0;
	
	/* remove existing key 
	NOT_FOUND or FOUND_ENTRY
	*/
	virtual OpResultTp remove(const KeyTp& key) = 0;

	struct EntryInfoTp {
		EntryInfoTp() {}
		EntryInfoTp(const KeyTp& _key, EntryIndexTp idx) : key(_key), index(idx) {}
		KeyTp key;
		EntryIndexTp index;
	};

	virtual EntryInfoTp envict() = 0;

	virtual std::vector<EntryInfoTp> get_data() = 0;

	virtual size_t size() = 0;
	virtual size_t capacity() = 0;
	
	virtual void resize_capacity(size_t new_cap) = 0;
	virtual void clear() = 0;
};


class LRU_Policy: public CachePolicyInterface {
public:
	LRU_Policy() :_capacity(0) {}
	LRU_Policy(size_t capacity) { assert(capacity > 0);  init(capacity); }

	OpResultTp check(const KeyTp& key);
	OpResultTp access(const KeyTp& key);
	OpResultTp update(const KeyTp& key);
	OpResultTp remove(const KeyTp& key);
	
	size_t size() { return map.size(); }
	size_t capacity() { return _capacity; }
	void clear();

	EntryInfoTp envict();
	std::vector<EntryInfoTp > get_data();

	void resize_capacity(size_t new_cap);
private:
	void init(size_t capacity);

	typedef std::list<KeyTp> AccessLstTp;

	AccessLstTp _access;
	std::vector<EntryIndexTp> freelst;
	std::unordered_map<KeyTp, std::pair<EntryIndexTp, AccessLstTp::iterator> > map;
	size_t _capacity;
};

template<typename CachePolicyInterface, typename TableTp>
class CacheTableGeneric {
};

}//namespace
