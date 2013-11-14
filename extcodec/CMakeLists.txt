cmake_minimum_required(VERSION 2.6)

project(extcodec)

#find_package(Boost COMPONENTS iostreams)
#if (MSVC)
#	set(Boost_LIBRARY_DIR C:/lib/boost_1_54_0/lib)
#	message("dir = " ${Boost_LIBRARY_DIR})
#	link_directories(${Boost_LIBRARY_DIR})
#endif()

include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SNAPPY_INCLUDE_DIRS})


set(SRCS cstream.cpp compression_streams.cpp mem_codec.cpp)

set(HEADERS cstream.h compression_streams.h mem_codec.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(extcodec ${SRCS} ${HEADERS})
target_link_libraries(extcodec utils ${SNAPPY_LIBRARY} ${ZLIB_LIBRARY} ${Boost_LIBRARIES})

add_executable(t_extcodec test_extcodec.cpp)
target_link_libraries(t_extcodec extcodec utils)
set_target_properties(t_extcodec PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

