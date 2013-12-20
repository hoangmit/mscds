
//#define BOOST_NETWORK_NO_LIB
#define BOOST_DATE_TIME_NO_LIB




#include <boost/network/protocol/http/client.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>

#include <unordered_map>

#include "strptime.h"

using namespace std;
using namespace boost::network;
using namespace boost::network::http;


struct HttpObject {
	HttpObject() {}

	HttpObject(const std::string& url) { set_url(url); }

	void set_url(const std::string& url) {
		this->url = url;
		http::client::options opt;
		opt.follow_redirects(true);
		client = http::client(opt);
	}

	void head() {
		http::client::request request(url);
		//request << header("Connection", "close");
		response = client.head(request);
		req_status = status(response);
	}

	void build_header_map() {
		auto hs = headers(response);
		for (auto& h : hs) {
			//name is not case-sensitivity
			std::string ss = h.first;
			std::transform(ss.begin(), ss.end(), ss.begin(), ::tolower);
			hdmap[ss] = h.second;
		}
	}

	std::string findheader_nocheck(const std::string& name) const {
		auto it = hdmap.find(name);
		if (it != hdmap.end()) return it->second;
		else return "";
	}

	void abc() {
	}

	std::string url;

	http::client client;
	http::client::response response;
	unsigned int req_status;

	std::unordered_map<std::string, std::string> hdmap;
};

void getinfo(const std::string& url) {

	http::client::options opt;

	opt.follow_redirects(true);
	http::client client(opt);
	http::client::request request(url);
	request << header("Connection", "close");
	request << header("Range", "bytes=0-0");
	http::client::response response = client.head(request);
	auto hs = headers(response);
	cout << status(response) << endl;
	//cout << status_message(response) << endl;
	for (auto h : hs) {
		cout << h.first << ' ' << h.second << endl;
	}
	cout << endl;

}

void test1() {

	/*if (argc != 2) {
	std::cout << "Usage: " << argv[0] << " [url]" << std::endl;
	return 1;
	}*/
	http::client::options opt;

	opt.follow_redirects(true);
	std::string url = "http://genome.ddns.comp.nus.edu.sg/~hoang";
	http::client client(opt);
	http::client::request request(url);
	request << header("Connection", "close");
	http::client::response response = client.get(request);
	auto hs = headers(response);
	for (auto h : hs) {
		cout << h.first << ' ' << h.second << endl;
	}
	cout << endl;
	//std::cout << esponse << std::endl;
	std::cout << body(response) << std::endl;

}

int main(int argc, char *argv[]) {
	getinfo("http://genome.ddns.comp.nus.edu.sg/~hoang");

	getinfo("http://genome.ddns.comp.nus.edu.sg/~hoang/bigWig/wgEncodeHaibTfbsGm12878RxlchPcr1xRawRep5.bigWig");

	getinfo("http://requestb.in/1gce6yb1");
    return 0;
}