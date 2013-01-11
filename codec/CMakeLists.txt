cmake_minimum_required(VERSION 2.6)

project(codec)

set(SRC
deltacoder.cpp
fibcoder.cpp
golomb_coder.cpp
)

set(HEADER
coder.h
deltacoder.h
fibcoder.h
golomb_coder.h
huffman_code.hpp
arithmetic_code.hpp
)

set(UNIT_TEST_SRC test_coder.cpp test_bstreamg.cpp test_arithmetic_huffman.cpp)

include(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
	message(FATAL_ERROR "Does not support big endian")
endif(${BIGENDIAN})


add_library(codec ${SRC} ${HEADER})
target_link_libraries(codec ${LIBS})


add_executable(t_codec ${UNIT_TEST_SRC} ${HEADER})
target_link_libraries(t_codec codec utils)


