
#include "utils/str_utils.h"
#include "http_client.h"
#include "error.h"
#include "strptime.h"

#define BOOST_DATE_TIME_NO_LIB
#include <boost/network/protocol/http/client.hpp>
//#include <boost/algorithm/string.hpp>

#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>

using namespace boost::network;
using namespace boost::network::http;
using namespace mscds;


struct body_handler {
	explicit body_handler(size_t _size, char* dest)
	: addr(dest), size(_size) {}

	BOOST_NETWORK_HTTP_BODY_CALLBACK(operator(), range, error) {
		if (!error) {
			for (const char c : range) {
				if (size == 0) break;
				*addr = c;
				++addr;
				--size;
			}
		}
	}

	char * addr;
	size_t size;
};

struct HttpObjectReq {
	typedef http::basic_client<http::tags::http_keepalive_8bit_udp_resolve, 1, 1> client_t;
	client_t client;

	HttpObjectReq() {}

	HttpObjectReq(const std::string& url) { set_url(url); }

	void set_url(const std::string& url) {
		this->url = url;
		client_t::options opt;
		opt.follow_redirects(true);
		client = client_t(opt);
	}

	void head() {
		client_t::request request(url);
		//request << header("Connection", "close");
		client_t::response response = client.head(request);
		req_status = status(response);
		parse_headers(response);
	}

	void getdata(size_t start, size_t len, char *dest, bool keepalive = true) {
		if (len == 0) return;
		client_t::request request(url);
		std::ostringstream os;
		os << "bytes=" << start << '-' << (start + len - 1);
		request << header("Range", os.str());
		if (keepalive)
			request << header("Connection", "Keep-Alive");
		client_t::response response = client.get(request); //body_handler(len, dest)
		req_status = status(response);
		if (req_status != 206)
			throw remoteio_error(std::string("wrong http status code: ") + utils::tostr(req_status) + "  (" + utils::tostr(start) + ","+utils::tostr(len)+")");
		parse_headers(response);
		size_t ctl = 0;
		if (content_length(ctl)) {
			if (ctl != len) throw remoteio_error("wrong length");
		}
		
		if (!content_range(last_range_info) || last_range_info.start == -1) throw remoteio_error("no content range");
		if (last_range_info.start != start || last_range_info.end != start + len - 1)
			throw remoteio_error("wrong range");

		std::string st = static_cast<std::string>(body(response));
		if (st.length() != len) throw remoteio_error("wrong length received");
		memcpy(dest, st.data(), len);
	}

	struct ContentRangeTp {
		int64_t start, end, total;
	};

	bool content_range(ContentRangeTp& ret) {
		if (s_content_range.empty()) return false;
		const char * p = s_content_range.c_str();
		const char * end = p + s_content_range.length();
		static const char bytes[] = "bytes";
		const char * bp = bytes;
		while (p < end && *bp && *p == *bp)
			{ ++p; ++bp; }
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
		} else {
			while (p < end && '0' <= *p && *p <= '9') {
				ret.total = ret.total * 10 + (*p - '0');
				++p;
			}
		}
		return true;
	}

	bool content_length(size_t& length) {
		if (s_content_length.empty())
			return false;
		length = atoll(s_content_length.c_str());
		return true;
	}

	bool last_modified(time_t& last_update) {
		if (s_last_modified.empty()) return false;
		struct tm tmx;
		if (strptime(s_last_modified.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tmx) == NULL) {
			throw remoteio_error("cannot parse last-modified string");
		}
		tmx.tm_isdst = -1;
		/*mktimeFromUtc*/
		last_update = mktime(&tmx);
		if (last_update == -1)
			throw remoteio_error("cannot convert time");
		return true;
	}

	bool date(time_t & rdate) {
		struct tm tmx;
		if (s_date.empty()) return false;
		if (strptime(s_date.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tmx) == NULL) {
			throw remoteio_error("cannot parse last-modified string");
		}
		tmx.tm_isdst = -1;
		/*mktimeFromUtc*/
		rdate = mktime(&tmx);
		if (rdate == -1)
			throw remoteio_error("cannot convert time");
		return true;
	}

	std::string s_content_length, s_content_range, s_last_modified, s_date, s_etag, s_connection;
	void parse_headers(client_t::response& response) {
		s_content_length.clear();
		s_content_range.clear();
		s_last_modified.clear();
		s_date.clear();
		auto hs = headers(response);
		//std::cout << "STATUS " << req_status << std::endl;
		for (auto& h : hs) {
			//name is not case-sensitivity
			std::string ss = h.first;
			std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);
			if ("content-length" == ss) {
				s_content_length = h.second;
			} else
			if ("content-range" == ss) {
				s_content_range = h.second;
			} else 
			if ("last-modified" == ss) {
				s_last_modified = h.second;
			} else
			if ("date" == ss) {
				s_date = h.second;
			}else 
			if ("etag" == ss) {
				s_etag = h.second;
			}else
			if ("connection" == ss) {
				s_connection = h.second;
			} else
			{}
			//std::cout << ss << " \t" << h.second << std::endl;
		}
		//std::cout << std::endl;
	}

	std::string url;

	unsigned int req_status;
	size_t total_size;
	ContentRangeTp last_range_info;
};

HttpFileObj::HttpFileObj(): impl(nullptr) {}


HttpFileObj::HttpFileObj(const std::string &url) {
	auto ret = std::make_shared<HttpObjectReq>(url);
	impl = ret.get();
	_impl_ctl = ret;
}

void HttpFileObj::getInfo(RemoteFileInfo &info) {
	HttpObjectReq * hobj = (HttpObjectReq *) impl;
	if (hobj == nullptr) throw remoteio_error("http object is not created");
	hobj->head();
	if (!hobj->content_length(info.filesize)) {
		try {
			char buff[2];
			hobj->getdata(0, 1, buff, false);
			info.filesize = hobj->last_range_info.total;
		} catch (remoteio_error&) {
			throw remoteio_error("unable to find file size");
		}
	}

	if (!hobj->last_modified(info.last_update))
		if (!hobj->date(info.last_update))
			throw remoteio_error("cannot find date or last_modified header");
}

void HttpFileObj::read_once(size_t start, size_t len, char *dest) {
	HttpObjectReq * hobj = (HttpObjectReq *) impl;
	if (hobj == nullptr) throw remoteio_error("http object is not initialized");
	hobj->getdata(start, len, dest);
}

void HttpFileObj::read_cont(size_t start, size_t len, char *dest) {
	HttpObjectReq * hobj = (HttpObjectReq *) impl;
	if (hobj == nullptr) throw remoteio_error("http object is not initialized");
	hobj->getdata(start, len, dest, true);
}

//------------------------------------------------------------------------

// Uri encode and decode.
// RFC1630, RFC1738, RFC2396

#include <string>
#include <assert.h>

const char HEX2DEC[128] =
{
	/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
	/* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	/* 3 */  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,

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
			char ch1 = *(pSrc + 1);
			char ch2 = *(pSrc + 2);
			if (ch1 < 128 && - 1 != (dec1 = HEX2DEC[ch1])
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
