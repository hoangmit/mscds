
#include "http_client.h"
#include "error.h"
#include "http_headers.h"


#include <boost/network/protocol/http/client.hpp>
//#include <boost/algorithm/string.hpp>

#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <functional>

using namespace boost::network;
using namespace boost::network::http;


struct HttpObjectReq {
	typedef std::function<void(boost::iterator_range<const char*> const &, boost::system::error_code const &)> BodyCallBack;

	typedef http::basic_client<http::tags::http_async_8bit_udp_resolve, 1, 1> client_t;

	client_t client;
	boost::shared_ptr<boost::asio::io_service> service_;
	std::string url;
	unsigned int req_status;
	//size_t total_size;
	
	HeaderInfo info_;


	HttpObjectReq() {}
	HttpObjectReq(const std::string& url) { init(url); }

	void init(const std::string& url) {
		this->url = url;
		restart();
	}

	void restart() {
		if (!service_) service_ = boost::make_shared<boost::asio::io_service>();
		client_t::options opt;
		opt.follow_redirects(true)
			.cache_resolved(true)
			.io_service(service_);
		client = client_t(opt);
	}

	void head() {
		client_t::request request(url);
		//request << header("Connection", "close");
		client_t::response response = client.head(request);
		req_status = status(response);
		check_headers(response);
	}

	void getdata(size_t start, size_t len, char *dest) {
		if (len == 0) return;
		client_t::request request(url);
		std::ostringstream os;
		os << "bytes=" << start << '-' << (start + len - 1);
		request << header("Range", os.str());
		client_t::response response = client.get(request); //body_handler(len, dest)
		req_status = status(response);
		check_req_info(start, len, response);
		std::string st = static_cast<std::string>(body(response));
		if (st.length() != len) throw remoteio_error("wrong length received");
		memcpy(dest, st.data(), len);
	}

	void getdata2(size_t start, size_t len, BodyCallBack callback) {
		if (len == 0) return;
		client_t::request request(url);
		std::ostringstream os;
		os << "bytes=" << start << '-' << (start + len - 1);
		request << header("Range", os.str());
		client_t::response response = client.get(request, callback);
		check_req_info(start, len, response);
	}

	void stop() {
		service_->stop();
		service_.reset();
	}
private:
	void check_headers(client_t::response& response) {
		info_.clear();
		auto hds = headers(response);
		for (auto& h : hds) info_.check_header(h.first, h.second);
	}

	void check_req_info(size_t start, size_t len, client_t::response& response) {
		if (req_status != 206)
			throw remoteio_error(std::string("wrong http status code: ") +
			std::to_string(req_status) + "  (" + std::to_string(start) + "," +
			std::to_string(len) + ")");
		
		check_headers(response);
		size_t ctl = 0;
		if (info_.content_length(ctl))
			if (ctl != len) throw remoteio_error("wrong length");

		if (!info_.content_range(info_.last_range_info) || info_.last_range_info.start == -1)
			throw remoteio_error("no content range");
		if (info_.last_range_info.start != start || info_.last_range_info.end != start + len - 1)
			throw remoteio_error("wrong range");
	}
};

//------------------------------------------------------------------------

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
	if (!hobj->info_.content_length(info.filesize)) {
		try {
			char buff[2];
			hobj->getdata(0, 1, buff);
			info.filesize = hobj->info_.last_range_info.total;
		} catch (remoteio_error&) {
			throw remoteio_error("unable to find file size");
		}
	}

	if (!hobj->info_.last_modified(info.last_update))
		if (!hobj->info_.date(info.last_update))
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
	hobj->getdata(start, len, dest);
}


