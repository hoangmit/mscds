
cmake_minimum_required(VERSION 2.6)

project(cwig)


set(SRCS stringarr.cpp)

set(HEADERS stringarr.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(string ${SRCS} ${HEADERS})
target_link_libraries(string bitarray intarray)

