cmake_minimum_required(VERSION 2.6)

project(codec)

set(SRC
deltacoder.cpp
fibcoder.cpp
golomb_coder.cpp
huffman_code.cpp
vbyte.cpp
)

set(HEADER
coder.h
deltacoder.h
fibcoder.h
golomb_coder.h
huffman_code.h
huffman_adp.hpp
arithmetic_code.hpp
vbyte.h
stream/codec_adapter.h
)



include(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
	message(FATAL_ERROR "Does not support big endian")
endif(${BIGENDIAN})


add_library(codec ${SRC} ${HEADER})
target_link_libraries(codec ${LIBS})

add_test_files(test_coder.cpp test_bstreamg.cpp test_arithmetic_huffman.cpp)


add_test_files(test_compression_framework.cpp)


#add_library(codec_stream codec_adapter.h)
#target_link_libraries(codec_stream codec bitarray)
