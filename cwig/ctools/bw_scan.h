
#pragma once

#ifndef __BW_SCAN_H_
#define __BW_SCAN_H_

#include <string>
class BigWigIntervals {
public:
	BigWigIntervals(){}
	void scan(const std::string& bwfile);
	// true --> cont, false -> skip this chromosome
	virtual bool start_chromsome(const std::string& s, unsigned int id, unsigned int size) {
		return true;
	}
	// this will not be called if start_chromsome skips
	virtual void end_chromosome() {}
	virtual void add_interval(unsigned int start, unsigned int end, double val) {}
};


#endif //__BW_SCAN_H_
