#pragma once

/**  \file

Least Recent Used cache table 

*/

#include <list>
#include <utility>
#include <unordered_map>
#include <cassert>
#include <vector>
#include <cstddef>
#include <stdint.h>

namespace utils {
/**
cache table model: a set of keys map to a limited size table. 
The table is cache_table[0..capacity-1],

This class helps to manage the organization of the table.
(It does not keep the actual table.)
E.g. Given a key: which entry index to access, which entry index to update
*/
class CacheTablePolicyInterface {
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

	/** check if table contains a key
	NOT_FOUND or FOUND_ENTRY
	*/
	virtual OpResultTp check(const KeyTp& key) = 0;
		
	/** find existing key, find a place to update,
	FOUND_ENTRY or REPLACED_ENTRY/NEW_ENTRY
	*/
	virtual OpResultTp access(const KeyTp& key) = 0;
	
	/** remove existing key 
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
	virtual size_t max_capacity() = 0;
	
	virtual void resize_capacity(size_t new_cap) = 0;
	virtual void clear() = 0;
};

/// Least Recent Use cache policy
class LRU_Policy : public CacheTablePolicyInterface {
public:
	LRU_Policy() :_capacity(0) {}
	LRU_Policy(size_t capacity) { assert(capacity > 0);  init(capacity); }

	OpResultTp check(const KeyTp& key);
	OpResultTp access(const KeyTp& key);
	OpResultTp remove(const KeyTp& key);
	
	size_t size() { return map.size(); }
	size_t capacity() { return _capacity; }
	size_t max_capacity();
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

/// Tree LRU policy
class TreePLRU_Policy : public CacheTablePolicyInterface {
public:
	TreePLRU_Policy();
	TreePLRU_Policy(size_t capacity) { assert(capacity > 0);  init(capacity); }

	OpResultTp check(const KeyTp& key);
	OpResultTp access(const KeyTp& key);
	OpResultTp remove(const KeyTp& key);

	size_t size() { return map.size(); }
	size_t capacity();
	size_t max_capacity();
	void clear();

	EntryInfoTp envict();
	std::vector<EntryInfoTp > get_data();

	void resize_capacity(size_t new_cap);
private:
	void init(size_t capacity);

	std::vector<KeyTp> list;
	std::vector<bool> direction;
	std::unordered_map<KeyTp, unsigned int> map;
};

/*
class TwowayAssociateCachePolicy : public CacheTablePolicyInterface {
public:
	TwowayAssociateCachePolicy() {}
	TwowayAssociateCachePolicy(size_t capacity) { assert(capacity > 0); init(capacity); }

	OpResultTp check(const KeyTp& key) {
		KeyTp cidx = key / (table.size() / 2);
		if (get(cidx, 0) == key)
			return OpResultTp(cidx * 2, FOUND_ENTRY);
		if (get(cidx, 1))
			return OpResultTp(cidx * 2 + 1, FOUND_ENTRY);
		return OpResultTp(0, NOT_FOUND);
	}
	OpResultTp access(const KeyTp& key) {
		KeyTp cidx = key / (table.size() / 2);
		if (get(cidx, 0) == key) {
			touch(cidx, 0);
			return OpResultTp(cidx * 2, FOUND_ENTRY);
		}
		if (get(cidx, 1)) {
			touch(cidx, 1);
			return OpResultTp(cidx * 2 + 1, FOUND_ENTRY);
		}
		if (is_free(cidx, 0)) {
			set(cidx, 0, key);
			return OpResultTp(cidx * 2, NEW_ENTRY);
		}
		if (is_free(cidx, 1)) {
			set(cidx, 1, key);
			return OpResultTp(cidx * 2 + 1, NEW_ENTRY);
		}
		unsigned int i = pull(cidx);
		set(cidx, i, key);
		return OpResultTp(cidx * 2 + 1, REPLACED_ENTRY);
	}

	OpResultTp remove(const KeyTp& key) {
		KeyTp cidx = key / (table.size() / 2);
		if (get(cidx, 0) == key) {
			free(cidx, 0);
			return OpResultTp(cidx * 2, FOUND_ENTRY);
		}
		if (get(cidx, 1)) {
			free(cidx, 1);
			return OpResultTp(cidx * 2 + 1, FOUND_ENTRY);
		}
	}

	size_t size() { return _size; }
	size_t capacity() { return table.size(); }
	size_t max_capacity();
	void clear();

	EntryInfoTp envict();
	std::vector<EntryInfoTp > get_data();

	void resize_capacity(size_t new_cap);
private:

	KeyTp get(unsigned int index, unsigned int way) { return table[index * 2 + way] & 0x7FFFFFFFu; }
	uint32_t set(unsigned int index, unsigned int way, KeyTp val) { table[index * 2 + way] = (table[index * 2 + way] & 0x80000000) | val; touch(index, way); }
	bool is_free(unsigned int index, unsigned int way) { return get(index, way) == EMPTY_CELL; }
	void free(unsigned int index, unsigned int way) { set(index, way, EMPTY_CELL); }
	void touch(unsigned int index, unsigned int way) { table[index * 2] = (table[index * 2] | 0x7FFFFFFFu) & (way << 31); }
	unsigned int pull(unsigned int index) { return 1 - (table[index * 2] >> 31); }

	static const uint32_t EMPTY_CELL = 0x7FFFFFFFu;
	void init(size_t capacity);

	std::vector<uint32_t> table;
	size_t _size;
};*/


}//namespace
