cmake_minimum_required(VERSION 2.6)

project(bitarray)

set(SRCS rank6p.cpp rank3p.cpp bitop.cpp bitarray.cpp bitstream.cpp rrr.cpp rrr2.cpp
select_dense.cpp
)
set(HEADERS bitarray.h rank6p.h rank3p.h rank25p.h rankselect.h bitop.h bitstream.h rrr.h rrr2.h
select_dense.h
bitrange.h
)

add_library(bitarray ${SRCS} ${HEADERS})
target_link_libraries(bitarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_test_files(ranktest.cpp select_test.cpp bit_test.cpp)

add_benchmark_files(bit_benchmark.cpp)

