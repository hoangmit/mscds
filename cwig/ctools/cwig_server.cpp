
#include <boost/network/include/http/server.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

namespace http = boost::network::http;
namespace utils = boost::network::utils;

struct request_server;
typedef http::async_server<request_server> server;

struct file_cache {
	boost::shared_mutex cache_mutex;
	std::string doc_root_;

	explicit file_cache(std::string const & doc_root)
		: doc_root_(doc_root) {}

	bool has(std::string const & path) {
		boost::shared_lock<boost::shared_mutex> lock(cache_mutex);
		return true;
	}

	bool add(std::string const & path) {
		return true;
	}
};

struct connection_handler : boost::enable_shared_from_this<connection_handler> {
	explicit connection_handler(file_cache & cache)
	: file_cache_(cache) {}

	void operator()(std::string const & path, server::connection_ptr connection, bool serve_body) {
		bool ok = file_cache_.has(path);
		if (!ok) ok = file_cache_.add(path);
		if (ok) {
			//send_headers(file_cache_.meta(path), connection);
			//if (serve_body) send_file(file_cache_.get(path), 0, connection);
		}
		else {
			not_found(path, connection);
		}
	}

	void not_found(std::string const & path, server::connection_ptr connection) {
		static server::response_header headers[] = {
			{ "Connection", "close" }
			, { "Content-Type", "text/plain" }
		};
		connection->set_status(server::connection::not_found);
		connection->set_headers(boost::make_iterator_range(headers, headers + 2));
		connection->write("File Not Found!");
	}

	template <class Range>
	void send_headers(Range const & headers, server::connection_ptr connection) {
		connection->set_status(server::connection::ok);
		connection->set_headers(headers);
	}

	void send_file(std::pair<void *, std::size_t> mmaped_region, off_t offset, server::connection_ptr connection) {
	}

	void handle_chunk(std::pair<void *, std::size_t> mmaped_region, off_t offset, server::connection_ptr connection, boost::system::error_code const & ec) {
		if (!ec && offset < mmaped_region.second) send_file(mmaped_region, offset, connection);
	}

	file_cache & file_cache_;
};

struct request_server {
	explicit request_server(file_cache & cache)
	: cache_(cache) {}

	void operator()(
		server::request const & request,
		server::connection_ptr connection
		) {
		if (request.method == "HEAD") {
			boost::shared_ptr<connection_handler> h(new connection_handler(cache_));
			(*h)(request.destination, connection, false);
		}
		else if (request.method == "GET") {
			boost::shared_ptr<connection_handler> h(new connection_handler(cache_));
			(*h)(request.destination, connection, true);
		}
		else {
			static server::response_header error_headers[] = {
				{ "Connection", "close" }
			};
			connection->set_status(server::connection::not_supported);
			connection->set_headers(boost::make_iterator_range(error_headers, error_headers + 1));
			connection->write("Method not supported.");
		}
	}

	void log(...) {
		// do nothing
	}

	file_cache & cache_;
};

int main(int argc, char * argv[]) {
	file_cache cache(".");
	request_server handler(cache);
	server::options options(handler);
	server instance(
		options.thread_pool(boost::make_shared<utils::thread_pool>(4))
		.address("0.0.0.0")
		.port("8000"));
	instance.run();
	return 0;
}
