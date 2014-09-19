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
set_property(TARGET cbed PROPERTY FOLDER "Apps")


add_test_exec(t_cbed FILES cbed_test.cpp LIBS cbed utils)
