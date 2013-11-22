cmake_minimum_required(VERSION 2.6)

project(cbed)

#find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(SRCS cbed.cpp blkcomp.cpp intv.cpp)

set(HEADERS cbed.h genomedata.h genomedata.hxx blkcomp.h intv.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(cbed ${SRCS} ${HEADERS})
target_link_libraries(cbed mscdsa extcodec)

add_executable(t_cbed cbed_test.cpp)
target_link_libraries(t_cbed cbed utils)
set_target_properties(t_cbed PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET t_cbed PROPERTY FOLDER "Tests")

