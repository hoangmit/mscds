cmake_minimum_required(VERSION 2.6)

project(extcodec)


include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SNAPPY_INCLUDE_DIRS})


set(SRCS cstream.cpp compression_streams.cpp mem_codec.cpp)

set(HEADERS cstream.h compression_streams.h mem_codec.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(extcodec ${SRCS} ${HEADERS})
target_link_libraries(extcodec  ${SNAPPY_LIBRARY} ${ZLIB_LIBRARY} ${Boost_LIBRARIES})

#add_test_files(test_extcodec.cpp)
add_test_exec(t_extcodec FILES test_extcodec.cpp LIBS extcodec utils)
