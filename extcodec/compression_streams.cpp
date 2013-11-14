#include "compression_streams.h"
#include <fstream>
#include <iostream>
//#include <stdexception>

#include <iosfwd>                          // streamsize
#include <boost/iostreams/categories.hpp>  // sink_tag
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp> 

//#include <boost/make_shared.hpp>


namespace io = boost::iostreams;


namespace mscds {

struct CountSinkTp {
	typedef char char_type;
	typedef io::sink_tag category;
	size_t chrcount;
	CountSinkTp() : chrcount(0){}
	std::streamsize write(const char* s, std::streamsize n)    {
		chrcount += n;
		return n;
	}
};

struct ImplBoost : public ImplCompress {
	CountSinkTp sink;
	io::filtering_ostream _out;
	ImplBoost(ExtCompMethod type) {
		switch (type) {
		case ZLIB:  _out.push(io::zlib_compressor()); break;
		case GZIP:  _out.push(io::gzip_compressor()); break;
		case BZIP2: _out.push(io::bzip2_compressor()); break;
		default:
			throw std::runtime_error("unknow compress type");
		}
		_out.push(boost::ref(sink));
	}
	std::ostream& out() { return _out; }
	size_t streamsize() { return sink.chrcount * sizeof(CountSinkTp::char_type); }
	void close() { _out.flush(); _out.reset(); }
};

struct ImplStoreBoost : public ImplCompress {
	io::filtering_ostream _out;
	std::vector<char> * buff;
	ImplStoreBoost(ExtCompMethod type, std::vector<char>* buff) {
		this->buff = buff;
		switch (type) {
		case ZLIB:  _out.push(io::zlib_compressor()); break;
		case GZIP:  _out.push(io::gzip_compressor()); break;
		case BZIP2: _out.push(io::bzip2_compressor()); break;
		default:
			throw std::runtime_error("unknow compress type");
		}
		_out.push(boost::iostreams::back_inserter(*buff));
	}
	std::ostream& out() { return _out; }
	size_t streamsize() { return buff->size(); }
	void close() { _out.flush(); _out.reset(); }
};

//-------------------------------------------------------------------------

size_estimate_stream::size_estimate_stream() : impl(NULL), streamsize(0) {}
size_estimate_stream::size_estimate_stream(ExtCompMethod type) {
	init_stream(type);
}

size_estimate_stream::~size_estimate_stream() {
	if (impl != NULL) { delete impl; impl = NULL; }
}

void size_estimate_stream::init_stream(ExtCompMethod type) {
	impl = new ImplBoost(type);
	streamsize = 0;
}

void size_estimate_stream::save(char c) {
	impl->out().put(c);
}

void size_estimate_stream::push_printf(unsigned int n, char sep) {
	impl->out() << n << sep;
}

void size_estimate_stream::save(uint16_t n) {
	impl->out().write((char*)&n, sizeof(n));
}

void size_estimate_stream::save(uint32_t n) {
	impl->out().write((char*)&n, sizeof(n));
}

void size_estimate_stream::save(uint64_t n) {
	impl->out().write((char*)&n, sizeof(n));
}

void size_estimate_stream::write(const char* str, size_t len) {
	impl->out().write((char*)str, len);
}

size_t size_estimate_stream::size() const {
	return streamsize;
}

void size_estimate_stream::close() {
	impl->close();
	this->streamsize = impl->streamsize();
	delete impl;
	impl = NULL;
}


//ImplGzip * impl;


}//namespace