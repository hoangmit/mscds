#pragma once

#include <string>

namespace mscds {

// interface for memory codec
class MemoryCodecMethod {
public:
	virtual size_t compress_c(const char* input, size_t input_length, std::string* output) = 0;
	size_t compress(const std::string& input, std::string* output) { return compress_c(input.c_str(), input.length(), output); }
	virtual bool uncompress_c(const char* compressed, size_t compressed_length, std::string* uncompressed) = 0;
	bool uncompress(const std::string& compressed, std::string* uncompressed) { return uncompress_c(compressed.c_str(), compressed.length(), uncompressed); }
};

class SnappyCodec : public MemoryCodecMethod {
public:
	size_t compress_c(const char* input, size_t input_length, std::string* output);
	bool uncompress_c(const char* compressed, size_t compressed_length, std::string* uncompressed);
};

class ZlibCodec : public MemoryCodecMethod {
public:
	ZlibCodec();
	size_t compress_c(const char* input, size_t input_length, std::string* output);
	bool uncompress_c(const char* compressed, size_t compressed_length, std::string* uncompressed);
private:
};


}//namespace