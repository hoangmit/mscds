cmake_minimum_required(VERSION 2.6)

project(codec)

set(SRC
deltacoder.cpp
fibcoder.cpp
golomb_coder.cpp
huffman_code.cpp
)

set(HEADER
coder.h
deltacoder.h
fibcoder.h
golomb_coder.h
huffman_code.h
huffman_adp.hpp
arithmetic_code.hpp
)



include(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
	message(FATAL_ERROR "Does not support big endian")
endif(${BIGENDIAN})


add_library(codec ${SRC} ${HEADER})
target_link_libraries(codec ${LIBS})

add_test_files(test_coder.cpp test_bstreamg.cpp test_arithmetic_huffman.cpp)
#set(UNIT_TEST_SRC test_coder.cpp test_bstreamg.cpp test_arithmetic_huffman.cpp)
#add_executable(t_codec ${UNIT_TEST_SRC} ${HEADER})
#target_link_libraries(t_codec codec utils)

add_executable(t_compression_fw test_compression_framework.cpp ${HEADER})
target_link_libraries(t_compression_fw codec utils intarray)
set_target_properties(t_compression_fw PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


