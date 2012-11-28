
#include "str_utils.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <cctype>
#include <iterator>
#include <sstream>
using namespace std;

namespace utils {

	using namespace std;

	// trim from start 
	inline std::string &ltrim_(std::string &s) { 
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace)))); 
		return s; 
	} 
 
	// trim from end 
	inline std::string &rtrim_(std::string &s) { 
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end()); 
		return s; 
	}

	// trim from both ends 
	std::string &trim_(std::string &s) { 
		return ltrim_(rtrim_(s)); 
	}

	std::string trim(const std::string& input) {
		std::string str = input;
		trim_(str);
		return str;
	}


	bool suffixstreq(const std::string& str, const std::string& suf) {
		if (str.length() < suf.length()) return false;
		else {
			for (unsigned int i = 0; i < suf.length(); i++)
				if (suf[i] != str[str.length() - suf.length() + i]) return false;
		}
		return true;
	}

	std::vector<std::string> tokenize(const std::string str) {
		stringstream strstr(str);
		istream_iterator<string> it(strstr), end; 
		return vector<string>(it, end);
	}

}

