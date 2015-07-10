#pragma once

/**  \file

Parse HTTP header for HTTP client.

*/

#include "error.h"
#include "strptime.h"

#include <boost/network/protocol/http/client.hpp>
#include <algorithm>
#include <string>
#include <stdint.h>
#include <ctime>


namespace mscds {

//typedef boost::network::http::basic_client<boost::network::http::tags::http_async_8bit_udp_resolve, 1, 1> client_t;

/// Encode string using URI scheme
std::string uri_encode(const std::string & sSrc);

/// Decode string from URI scheme
std::string uri_decode(const std::string & sSrc);

struct ContentRangeTp {
	int64_t start, end, total;
};

/// parse HTTP headers
struct HeaderInfo {

	std::string s_content_length, s_content_range, s_last_modified, s_date, s_etag, s_connection;

	ContentRangeTp last_range_info;

	bool content_range(ContentRangeTp& ret);

	bool content_length(size_t& length);

	bool last_modified(time_t& last_update);

	bool date(time_t & rdate);

	void clear();

	void check_header(const std::string& name, const std::string& value);


};

}//namespace