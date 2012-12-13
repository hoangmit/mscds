
#ifndef __UTILS_DEBUG_H_
#define __UTILS_DEBUG_H_

#include <string>
#include <iostream>

namespace utils {

struct Debugger {
	static bool flag;
	Debugger(){ flag = false; }
	void print(const std::string& s) {
		if (flag)
			std::cout << s << std::endl;
	}

	bool is_on() { return flag; }
	void setflag(bool b) { flag = b; }
};


}//namespace
#endif //__UTILS_DEBUG_H_