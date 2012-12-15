
#ifndef __UTILS_DEBUG_H_
#define __UTILS_DEBUG_H_

#include <string>
#include <iostream>

namespace utils {

struct DbgHelper {
	static bool flag;
	DbgHelper(){ flag = false; }
	void print(const std::string& s) {
		if (flag) std::cerr << s << std::endl;
	}

	bool is_on() { return flag; }
	void setflag(bool b) { flag = b; }
};


}//namespace
#endif //__UTILS_DEBUG_H_