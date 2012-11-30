#pragma once


#include <stdint.h>
#include <cassert>
#include <string>
#include <ctime>

/** \brief ultility functions

provides commonly used functions. There are 3 major groups of functions provided
(1) group of functions to manage time
(2) group of functions to manipulate string
(3) group of function to handle file
 */
namespace utils {	
	/** \brief returns the ceiling of "a" over "b" */
	inline uint64_t ceildiv(uint64_t a, uint64_t b) {
		return (a + b - 1) / b;
	}

	/** \brief time in milli-seconds */
	uint64_t getTimeMs64();
		
	/** \brief Stopwatch class to measure running time */
	class Stopwatch {
	public:
		Stopwatch();
		void start();
		void stop();
		/** \brief return the time from the last start() call in seconds */
		double seconds();
		uint64_t start_time, stop_time;
	};

	
}
