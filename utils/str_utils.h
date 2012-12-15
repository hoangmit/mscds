#pragma once

#ifndef __STR_UTILS_H_
#define __STR_UTILS_H_

#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <utility>
#include <stdint.h>

namespace utils {

	/*! \brief adds "/" between prefix and suffix to make it a valid path */
	std::string joinp(const std::string& prefix, const std::string& suffix);
	/*! \brief returns a new trimed string on both left and right of the input */
	std::string trim(const std::string& input);
	
	/*! \brief trims input string and return the modified one */
	std::string & trim_(std::string &s);

	/*! \brief a generic function that converts variable "v" into string to display */
	template<typename T>
	inline std::string tostr(const T& v) {
		std::ostringstream ss;
		ss << v;
		return ss.str();
	}

	/*! \brief a generic function that converts string "s" into variable of type "T". It may throw runntime_error if the string cannot be converted */
	template<typename T>
	inline T strto(const std::string& s) {
		std::istringstream ss(s);
		T v;
		ss >> v;
		if (!ss) {
			std::string msg = std::string("conversion error: input=\"")+s+"\"";
			throw std::runtime_error(msg);
		}
		return v;
	}

	/*! \brief the specialized function of "strto" for type "unsigned int" */
	template<>
	inline unsigned int strto<unsigned int>(const std::string& s) {
		unsigned int ret = 0;
		size_t i = 0, start, end;
		start = 0;
		while (start < s.length() && isspace(s[start])) start++;
		end = s.length(); 
		while (end > 0 && isspace(s[end - 1])) end--;
		if (start >= end) throw std::runtime_error("conversion error");
		if (s[start] == '+') start += 1;
		for (i = start; i < end; ++i) {
			char c = s[i];
			if ('0' <= c && c <= '9') ret = ret * 10 + (c - '0');
			else throw std::runtime_error("conversion error");
		}
		return ret;
	}

	/*! \brief the specialized function of "strto" for type "uint64_t" */
	template<>
	inline uint64_t strto<uint64_t>(const std::string& s) {
		uint64_t ret = 0;
		size_t i = 0, start, end;
		start = 0;
		while (start < s.length() && isspace(s[start])) start++;
		end = s.length(); 
		while (end > 0 && isspace(s[end - 1])) end--;
		if (start >= end) throw std::runtime_error("conversion error");
		if (s[start] == '+') start += 1;
		for (i = start; i < end; ++i) {
			char c = s[i];
			if ('0' <= c && c <= '9') ret = ret * 10 + (c - '0');
			else throw std::runtime_error("conversion error");
		}
		return ret;
	}

	/*! \brief checks if the input string has the given suffix */
	bool suffixstreq(const std::string& str, const std::string& suf);

	/*! \brief tokenizes the input string into a set of strings. The delimiter is space characters */
	std::vector<std::string> tokenize(const std::string str);

	/** \brief returns 64 bits in binary string of input "n" for testing purpose */
	inline std::string binstr(uint64_t n) {
		std::ostringstream ss;
		for (int i = 0; i < 64; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
			if (n == 0) break;
		}
		return ss.str();
	}
	/** \brief returns "len" bits in binary string of input "n" for testing purpose */
	inline std::string binstr(uint64_t n, int len) {
		std::ostringstream ss;
		for (int i = 0; i < len; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
		}
		return ss.str();
	}

	/** \brief same as binstr(uint64_t n, int len) but in a different input format */
	template<typename T>
	inline std::string binstr(const std::pair<uint64_t, T>& p) {
		std::ostringstream ss;
		uint64_t n = p.first;
		for (int i = 0; i < p.second; ++i) {
			ss << (int)(n & 1);
			n = n >> 1;
		}
		return ss.str();
	}

	#define WATCH(x) cout<< #x <<"="<<(x)<<endl;
	
	template <class cT, class traits = std::char_traits<cT> >
	class basic_nullbuf: public std::basic_streambuf<cT, traits> {
		typename traits::int_type overflow(typename traits::int_type c) {
			return traits::not_eof(c); // indicate success
		}
	};

	template <class cT, class traits = std::char_traits<cT> >
	class basic_onullstream: public std::basic_ostream<cT, traits> {
	public:
		basic_onullstream():
			std::basic_ios<cT, traits>(&m_sbuf),
			std::basic_ostream<cT, traits>(&m_sbuf)
		{
			init(&m_sbuf);
		}
	private:
		basic_nullbuf<cT, traits> m_sbuf;
	};
	/*! \brief empty output stream */
	typedef basic_onullstream<char> onullstream;

	class comma_numpunct : public std::numpunct<char> {
	protected:
		virtual char do_thousands_sep() const { return ','; }
		virtual std::string do_grouping() const { return "\03"; }
	};
	//locale oldLoc = cout.imbue(locale(cout.getloc(), new comma_numpunct()));
}

#endif //__STR_UTILS_H_
