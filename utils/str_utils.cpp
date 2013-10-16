
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

// from http://tinodidriksen.com/ and http://www.leapsecond.com/tools/fast_atof.c
int atoi2(const char *p) {
	int x = 0;
	bool neg = false;
	if (*p == '-') {
		neg = true;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		x = (x*10) + (*p - '0');
		++p;
	}
	if (neg) {
		x = -x;
	}
	return x;
}



double atof3(const char *p) {
	double r = 0.0;
	bool neg = false;
	if (*p == '-') {
		neg = true;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		r = (r*10.0) + (*p - '0');
		++p;
	}
	if (*p == '.') {
		double f = 0.0;
		int n = 0;
		++p;
		while (*p >= '0' && *p <= '9') {
			f = (f*10.0) + (*p - '0');
			++p;
			++n;
		}
		r += f / std::pow(10.0, n);
	}
	if (neg) {
		r = -r;
	}
	return r;
}


#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

double atof2(const char *p)
{
	int frac;
	double sign, value, scale;

	// Skip leading white space, if any.

	while (white_space(*p) ) {
		p += 1;
	}

	// Get sign, if any.

	sign = 1.0;
	if (*p == '-') {
		sign = -1.0;
		p += 1;

	} else if (*p == '+') {
		p += 1;
	}

	// Get digits before decimal point or exponent, if any.

	for (value = 0.0; valid_digit(*p); p += 1) {
		value = value * 10.0 + (*p - '0');
	}

	// Get digits after decimal point, if any.

	if (*p == '.') {
		double pow10 = 10.0;
		p += 1;
		while (valid_digit(*p)) {
			value += (*p - '0') / pow10;
			pow10 *= 10.0;
			p += 1;
		}
	}

	// Handle exponent, if any.

	frac = 0;
	scale = 1.0;
	if ((*p == 'e') || (*p == 'E')) {
		unsigned int expon;

		// Get sign of exponent, if any.

		p += 1;
		if (*p == '-') {
			frac = 1;
			p += 1;

		} else if (*p == '+') {
			p += 1;
		}

		// Get digits of exponent, if any.

		for (expon = 0; valid_digit(*p); p += 1) {
			expon = expon * 10 + (*p - '0');
		}
		if (expon > 308) expon = 308;

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >=  8) { scale *= 1E8;  expon -=  8; }
		while (expon >   0) { scale *= 10.0; expon -=  1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}




}//namespace

