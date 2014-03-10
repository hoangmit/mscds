#include "http_headers.h"

#include "error.h"

// Uri encode and decode.
// RFC1630, RFC1738, RFC2396

#include <string>
#include <assert.h>

using namespace mscds;

const char HEX2DEC[128] =
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 3 */  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,

	/* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

};

std::string uri_decode(const std::string & sSrc)
{
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const size_t SRC_LEN = sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC) {
		if (*pSrc == '%') {
			char dec1, dec2;
			unsigned char ch1 = *(pSrc + 1);
			unsigned char ch2 = *(pSrc + 2);
			if (ch1 < 128 && -1 != (dec1 = HEX2DEC[ch1])
				&& ch2 < 128 && -1 != (dec2 = HEX2DEC[ch2]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
		*pEnd++ = *pSrc++;

	std::string sResult(pStart, pEnd);
	delete[] pStart;
	return sResult;
}

// Only alphanum is safe.
const char SAFE[128] =
{
	/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
	/* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

	/* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
	/* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0
};

std::string uri_encode(const std::string & sSrc)
{
	static const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const size_t SRC_LEN = sSrc.length();
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc) {
		if (*pSrc < 128 && SAFE[*pSrc])
			*pEnd++ = *pSrc;
		else {
			// escape this char
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
	}

	std::string sResult((char *)pStart, (char *)pEnd);
	delete[] pStart;
	return sResult;
}

//------------------------------------------------------------------------

bool HeaderInfo::content_range(ContentRangeTp &ret) {
	if (s_content_range.empty()) return false;
	const char * p = s_content_range.c_str();
	const char * end = p + s_content_range.length();
	static const char bytes[] = "bytes";
	const char * bp = bytes;
	while (p < end && *bp && *p == *bp)
	{
		++p; ++bp;
	}
	if (*bp != '\0') throw remoteio_error("expected bytes range");

	while (p < end && *p == ' ') ++p;
	if (*p == '*') {
		ret.start = -1;
		ret.end = -1;
	}
	else {
		ret.start = 0;
		ret.end = 0;
		ret.total = 0;
		while (p < end && '0' <= *p && *p <= '9') {
			ret.start = ret.start * 10 + (*p - '0');
			++p;
		}
		if (p >= end || *p != '-') throw remoteio_error("error parsing range");
		++p;
		while (p < end && '0' <= *p && *p <= '9') {
			ret.end = ret.end * 10 + (*p - '0');
			++p;
		}
	}
	if (p >= end || *p != '/') throw remoteio_error("error parsing range");
	++p;
	if (*p == '*') {
		ret.total = -1;
	}
	else {
		while (p < end && '0' <= *p && *p <= '9') {
			ret.total = ret.total * 10 + (*p - '0');
			++p;
		}
	}
	return true;
}

bool HeaderInfo::content_length(size_t &length) {
	if (s_content_length.empty())
		return false;
	length = atoll(s_content_length.c_str());
	return true;
}

bool HeaderInfo::last_modified(time_t &last_update) {
	if (s_last_modified.empty()) return false;
	struct tm tmx;
	if (strptime(s_last_modified.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tmx) == nullptr) {
		throw remoteio_error("cannot parse last-modified string");
	}
	tmx.tm_isdst = -1;
	/*mktimeFromUtc*/
	last_update = mktime(&tmx);
	if (last_update == -1)
		throw remoteio_error("cannot convert time");
	return true;
}

bool HeaderInfo::date(time_t &rdate) {
	struct tm tmx;
	if (s_date.empty()) return false;
	if (strptime(s_date.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tmx) == nullptr) {
		throw remoteio_error("cannot parse last-modified string");
	}
	tmx.tm_isdst = -1;
	/*mktimeFromUtc*/
	rdate = mktime(&tmx);
	if (rdate == -1)
		throw remoteio_error("cannot convert time");
	return true;
}

void HeaderInfo::clear() {
	s_content_length.clear();
	s_content_range.clear();
	s_last_modified.clear();
	s_date.clear();
}

void HeaderInfo::check_header(const std::string &name, const std::string &value) {
	//name is not case-sensitivity
	std::string lname = name;
	std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
	if ("content-length" == lname) {
		s_content_length = value;
	}
	else
	if ("content-range" == lname) {
		s_content_range = value;
	}
	else
	if ("last-modified" == lname) {
		s_last_modified = value;
	}
	else
	if ("date" == lname) {
		s_date = value;
	}
	else
	if ("etag" == lname) {
		s_etag = value;
	}
	else
	if ("connection" == lname) {
		s_connection = value;
	}
	else
	{
	}
	//std::cout << ss << " \t" << h.second << std::endl;

}
