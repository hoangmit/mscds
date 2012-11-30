#pragma once


#include <stdint.h>
#include <cassert>
#include <string>
#include <ctime>

/** \brief ultility functions

provides commonly used functions. There are 3 major groups of functions provided
(1) group of functions to manipulate bits and bits arrays 
(2) group of functions to manipulate string
(3) group of function to handle file
 */
namespace utils {
	/** \brief takes "len" bits starts from bitindex to (bitindex+len-1), packs them into a 64bit uint and returns the answer */
	uint64_t get_bits_arr(const uint64_t * ptr, uint64_t bitindex, unsigned int len);
	
	/** \brief takes "len" bits in the "value" input variable, puts the bits into the bit array */ 
	void set_bits_arr(uint64_t * ptr, uint64_t bitindex, uint64_t value, unsigned int len);
	
	/** \brief returns a single bit in the bit array pointed by "ptr" */
	bool get_bit(const uint64_t * ptr, uint64_t bitindex);
	
	/** \brief set a single bit in the bit array pointed by "ptr" */
	void set_bit(uint64_t * ptr, uint64_t bitindex, bool value);
	
	/** \brief returns the ceiling of "a" over "b" */
	inline uint64_t ceildiv(uint64_t a, uint64_t b) {
		return (a + b - 1) / b;
	}
		
	/** \brief Stopwatch class to measure running time */
	class Stopwatch {
	public:
		void start();
		/** \brief return the time from the last start() call in seconds */
		double stop();
	private:
		std::clock_t st_time;
	};	
}
