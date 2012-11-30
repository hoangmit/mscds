
#include "utils.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <fstream>
using namespace std;
#include "str_utils.h"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif



namespace utils {

	Stopwatch::Stopwatch(): start_time(0), stop_time(0) {}

	void Stopwatch::start() {
		start_time = getTimeMs64();
	}
	
	void Stopwatch::stop() {	
		stop_time = getTimeMs64();
	}

	double Stopwatch::seconds() {
		return (double)(stop_time-start_time)/1000.0;
	}

	uint64_t getTimeMs64() {
	#ifdef WIN32
		/* Windows */
		FILETIME ft;
		LARGE_INTEGER li;

		/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
		* to a LARGE_INTEGER structure. */
		GetSystemTimeAsFileTime(&ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		uint64_t ret = li.QuadPart;
		ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
		ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

		return ret;
	#else
		/* Linux */
		struct timeval tv;
		gettimeofday(&tv, NULL);
		uint64_t ret = tv.tv_usec;
		/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
		ret /= 1000;
		/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
		ret += (tv.tv_sec * 1000);
		return ret;
	#endif
	}

}
