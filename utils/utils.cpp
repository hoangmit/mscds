
#include "utils.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <fstream>
using namespace std;
#include "str_utils.h"




namespace utils {
	uint64_t get_bits_arr(const uint64_t * ptr, uint64_t bitindex, unsigned int len)  {
		const unsigned int WORDLEN = 64;
		if (!(len <= WORDLEN && len > 0))
			throw runtime_error("invalid length input");
		assert(len <= WORDLEN && len > 0);
		uint64_t i = bitindex / WORDLEN;
		unsigned int j = bitindex % WORDLEN; 
		//assert(i < size());
		uint64_t mask;
		if (len < WORDLEN) mask = ((1ull << len) - 1); // (~0ull >> (WORDLEN - len));
		else mask = ~0ull;
		if (j + len <= WORDLEN) {
			//assert(i < size());
			return (ptr[i] >> j) & mask;
		} else {
			//assert(i + 1 < size());
			return (ptr[i] >> j) | ((ptr[i + 1] << (WORDLEN - j)) & mask);
		}
	}

	bool get_bit(const uint64_t * ptr, uint64_t bitindex) {
		const unsigned int WORDLEN = 64;
		return (ptr[bitindex / WORDLEN] & (1ULL << (bitindex % WORDLEN))) != 0;
	}

	void set_bit(uint64_t * ptr, uint64_t bitindex, bool value) {
		const unsigned int WORDLEN = 64;
		if (value)
			ptr[bitindex / WORDLEN] |= (1ULL << (bitindex % WORDLEN));
		else
			ptr[bitindex / WORDLEN] &= ~(1ULL << (bitindex % WORDLEN));
	}

	void set_bits_arr(uint64_t * ptr, uint64_t bitindex, uint64_t value, unsigned int len) {
		const unsigned int WORDLEN = 64;
		assert(len <= WORDLEN && len > 0);
		uint64_t i = bitindex / WORDLEN;
		unsigned int j = bitindex % WORDLEN; 
		//assert(i < size());
		uint64_t mask = ((1ull << len) - 1); // & (~0ull >> (WORDLEN - len))
		value = value & mask;
		if (j + len <= WORDLEN) {
			//assert(i < size());
			ptr[i] = (ptr[i] & ~(mask << j)) | (value << j);
		} else {
			//assert(i + 1 < size());
			ptr[i] = (ptr[i] & ~(mask << j)) | (value << j);
			ptr[i+1] = ptr[i+1] & ~ (mask >> (WORDLEN - j)) &  value >> (WORDLEN - j);
		}
	}

	uint64_t ceildiv(uint64_t a, uint64_t b) {
		if (b == 0) throw std::runtime_error("division by zero");
		if (a == 0) return 0;
		return ((a - 1) / b) + 1;
	}


	void Stopwatch::start() {
		st_time = clock();
	}
	
	double Stopwatch::stop() {	
		return (clock() - st_time)*1.0 / CLOCKS_PER_SEC;
	}	

}
