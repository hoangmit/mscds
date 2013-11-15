#include "mem_codec.h"

#include "snappy.h"
#include "zlib.h"

#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstring>

namespace mscds {

size_t SnappyCodec::compress_c(const char* input, size_t input_length, std::string* output) const {
	return snappy::Compress(input, input_length, output);
}

bool SnappyCodec::uncompress_c(const char* compressed, size_t compressed_length, std::string* uncompressed) const {
	return snappy::Uncompress(compressed, compressed_length, uncompressed);
}

size_t ZlibCodec::compress_c(const char* input, size_t input_length, std::string* output) const {
	int compressionlevel = Z_DEFAULT_COMPRESSION;
	z_stream zs;
	memset(&zs, 0, sizeof(zs));
	if (deflateInit(&zs, compressionlevel) != Z_OK)
		throw(std::runtime_error("deflateInit failed while compressing."));
	zs.next_in = (Bytef*) input;
	zs.avail_in = (uInt) input_length;
	int ret;
	char outbuffer[32768];
	output->clear();
	// retrieve the compressed bytes blockwise
	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);
		ret = deflate(&zs, Z_FINISH);
		if (output->size() < zs.total_out) {
			output->append(outbuffer,
				zs.total_out - output->size());
		}
	} while (ret == Z_OK);
	deflateEnd(&zs);
	if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
		std::ostringstream oss;
		oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
		throw(std::runtime_error(oss.str()));
	}
	return output->size();
}

bool ZlibCodec::uncompress_c(const char* compressed, size_t compressed_length, std::string* uncompressed) const {
	z_stream zs;
	memset(&zs, 0, sizeof(zs));
	if (inflateInit(&zs) != Z_OK) return false;
	zs.next_in = (Bytef*)compressed;
	zs.avail_in = (uInt)compressed_length;
	int ret;
	char outbuffer[32768];
	uncompressed->clear();
	do {
		zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
		zs.avail_out = sizeof(outbuffer);
		ret = inflate(&zs, 0);
		if (uncompressed->size() < zs.total_out) {
			uncompressed->append(outbuffer,
				zs.total_out - uncompressed->size());
		}
	} while (ret == Z_OK);
	inflateEnd(&zs);
	if (ret != Z_STREAM_END) return false;
	return true;
}

ZlibCodec::ZlibCodec() {}



}//namespace
