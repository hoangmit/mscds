
#include "url_parser.h"
#include "utils/file_utils.h"
#include "cwig/cwig.h"

#include <boost/network/include/http/server.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <unordered_map>
#include <sstream>
#include <stdexcept>

namespace http = boost::network::http;
namespace network = boost::network;

struct request_server;
typedef http::async_server<request_server> server;

struct file_cache {
	boost::shared_mutex cache_mutex;
	std::string doc_root_;
	unsigned int max_size;

	typedef std::list<std::pair<std::string, app_ds::GenomeNumData> > cache_t;
	cache_t cache;
	std::unordered_map<std::string, cache_t::iterator> table;


	explicit file_cache(std::string const & doc_root)
		: doc_root_(doc_root) {}

	bool has(std::string const & path) {
		boost::shared_lock<boost::shared_mutex> lock(cache_mutex);
		if (table.find(path) == table.end())
			return false;
		else
			return true;
	}

	bool load(std::string const & path) {
		boost::upgrade_lock<boost::shared_mutex> lock(cache_mutex);
		auto it = table.find(path);
		if (it != table.end()) return true;
		if (!utils::file_exists(doc_root_ + path)) return false;
		boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
		if (table.size() >= max_size) {
			clear_();
		}
		cache.emplace_front(path, app_ds::GenomeNumData());
		auto pos = table.insert(std::make_pair(path, cache.begin()));
		assert(pos.second);
		try {
			cache.begin()->second.loadfile(doc_root_ + path);
		} catch (std::runtime_error& ) {
			cache.erase(pos.first->second);
			table.erase(pos.first);
			return false;
		}
		return true;
	}

	template<typename T>
	void json_dumps(const std::vector<T>& v, std::ostream& out) {
		out << "[";
		if (v.size() > 0) {
			out << v[0];
			auto it = v.cbegin();
			++it;
			while (it != v.cend()) {
				out << ", " << *it;
				++it;
			}
		}
		out << "]";
	}

	bool get(const app_ds::chrom_intv_op& query, std::string& out) {
		boost::upgrade_lock<boost::shared_mutex> lock(cache_mutex);
		out.clear();
		auto it = table.find(query.file);
		if (it == table.end()) {
			out = "Not cached";  return false;
		}
		app_ds::GenomeNumData& qs = it->second->second;
		std::ostringstream ss;
		int chr = qs.getChrId(query.chrom);
		if (query.winsize == 0 || query.winsize > 10000) {
			out = "Too many data points";  return false;
		}
		if (chr < 0) { out = "Unknown chromosome name"; return false; }
		if (query.opname == "avg") {
			json_dumps(qs.getChr(chr).avg_batch(query.start, query.end, query.winsize), ss);
		}else
		if (query.opname == "cov") {
			json_dumps(qs.getChr(chr).coverage_batch(query.start, query.end, query.winsize), ss);
		} else
		if (query.opname == "min") {
			json_dumps(qs.getChr(chr).min_value_batch(query.start, query.end, query.winsize), ss);
		} else
		if (query.opname == "max") {
			json_dumps(qs.getChr(chr).max_value_batch(query.start, query.end, query.winsize), ss);
		}
		else {
			out = "Unknown operation";
			return false;
		}
		out = ss.str();
		return true;
	}
private:
	void clear_() {
		unsigned int sz = table.size() / 2;
		auto it = table.begin();
		while (it != table.end()) {
			if (sz == 0) break;
			if (rand() % 2 == 1) {
				cache.erase(it->second);
				table.erase(it++);
			} else {
				++it;
			}
		}
		while (table.size() > sz) {
			cache.erase(table.begin()->second);
			table.erase(table.begin());
		}
	}
};

struct connection_handler : boost::enable_shared_from_this<connection_handler> {
	explicit connection_handler(file_cache & cache, bool verbose=true)
	: file_cache_(cache), verbose_(verbose) {}

	void operator()(std::string const & path, server::connection_ptr connection) {
		if (verbose_)
			std::cout << path << " ";
		app_ds::chrom_intv_op res;
		bool ok = app_ds::parse_url_query(path, res);
		if (!ok) { error(connection, "Wrong query format"); return; }
		ok = file_cache_.has(res.file);
		if (!ok)
			ok = file_cache_.load(res.file);
		if (ok) {
			std::string outx;
			ok = file_cache_.get(res, outx);
			if (!ok) error(connection, outx);
			else success(connection, outx);
		}
		else {
			not_found(connection);
		}
	}

	void success(server::connection_ptr connection, const std::string& data) {
		static server::response_header headers[] = {
			{ "Connection", "close" }
			, { "Content-Type", "text/plain" }
		};
		connection->set_status(server::connection::ok);
		connection->set_headers(boost::make_iterator_range(headers, headers + 2));
		connection->write(data);
		if (verbose_)
			std::cout << " OK" << std::endl;
	}

	void error(server::connection_ptr connection, const std::string& msg) {
		static server::response_header headers[] = {
			{ "Connection", "close" }
			, { "Content-Type", "text/plain" }
		};
		connection->set_status(server::connection::internal_server_error);
		connection->set_headers(boost::make_iterator_range(headers, headers + 2));
		connection->write("Error: " + msg);
		if (verbose_)
			std::cout << " ERR" << std::endl;
	}

	void not_found(server::connection_ptr connection) {
		static server::response_header headers[] = {
			{ "Connection", "close" }
			, { "Content-Type", "text/plain" }
		};
		connection->set_status(server::connection::not_found);
		connection->set_headers(boost::make_iterator_range(headers, headers + 2));
		connection->write("File Not Found!");
		if (verbose_)
			std::cout << " NOT_FOUND" << std::endl;
	}

	bool verbose_;
	file_cache & file_cache_;
};

struct request_server {
	explicit request_server(file_cache & cache, bool verbose = true)
	: cache_(cache), verbose_(verbose) {}

	void operator()(
		server::request const & request,
		server::connection_ptr connection
		) {
		if (request.method == "GET") {
			boost::shared_ptr<connection_handler> h(new connection_handler(cache_, verbose_));
			(*h)(request.destination, connection);
		} else {
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

	bool verbose_;
	file_cache & cache_;
};

int main(int argc, char * argv[]) {
	file_cache cache("./");
	std::cout << "Starting server... ";
	request_server handler(cache, false);
	server::options options(handler);
	server instance(
		options.thread_pool(boost::make_shared<network::utils::thread_pool>(4))
		.address("0.0.0.0")
		.port("8000"));
	std::cout << std::endl;
	instance.run();
	return 0;
}

