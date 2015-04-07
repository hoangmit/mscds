
cmake_minimum_required(VERSION 2.6)

project(string_struct)


set(SRCS stringarr.cpp blob_array.cpp)

set(HEADERS stringarr.h blob_array.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(string ${SRCS} ${HEADERS})
target_link_libraries(string bitarray intarray)

add_test_files(stringarr_test.cpp)
