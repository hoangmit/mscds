cmake_minimum_required(VERSION 2.6)

project(codec)

set(SRC
deltacoder.cpp
fibcoder.cpp
golomb_coder.cpp
huffman_code.cpp
vbyte.cpp
rrr_codec.cpp
)

set(HEADER
coder.h
deltacoder.h
fibcoder.h
golomb_coder.h
huffman_code.h
huffman_adp.hpp
arithmetic_code.hpp
ac_models.h
aux_codec.h
vbyte.h
utf8.h
stream/codec_adapter.h
rrr_codec.h
)



include(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
	message(FATAL_ERROR "Does not support big endian")
endif(${BIGENDIAN})


add_library(codec ${SRC} ${HEADER})
target_link_libraries(codec ${LIBS})

add_test_files(test_coder.cpp test_bstreamg.cpp test_huffman.cpp test_aux_stream.cpp)


add_test_files(test_compression_framework.cpp test_rrr_code.cpp test_arithmetic_code.cpp)



#add_test_exec(t_arithmetic_code FILES test_arithmetic_code.cpp LIBS utils bitarray codec)
#add_library(codec_stream codec_adapter.h)
#target_link_libraries(codec_stream codec bitarray)


#add_test_exec(t_rrr_code FILES test_rrr_code.cpp LIBS utils bitarray codec)
