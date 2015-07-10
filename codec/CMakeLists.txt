cmake_minimum_required(VERSION 2.6)

project(codec)

set(SRC
deltacoder.cpp
fibcoder.cpp
golomb_coder.cpp
huffman_code.cpp
vbyte.cpp
rrr_codec.cpp
sym_table.cpp
sym_table_alias.cpp
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
rans_byte.hpp
sym_table.h
sym_table_alias.h
)



include(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
	message(FATAL_ERROR "Does not support big endian")
endif(${BIGENDIAN})


add_library(codec ${SRC} ${HEADER})
target_link_libraries(codec ${LIBS})

add_test_files(test_coder.cpp test_bstreamg.cpp test_huffman.cpp test_aux_stream.cpp)


add_test_files(test_compression_framework.cpp test_rrr_code.cpp test_arithmetic_code.cpp test_rans.cpp)



#add_library(codec_stream codec_adapter.h)
#target_link_libraries(codec_stream codec bitarray)


#add_test_exec(t_arithmetic_code FILES test_arithmetic_code.cpp LIBS utils codec)
#add_test_exec(t_rans FILES test_rans.cpp LIBS utils codec)
