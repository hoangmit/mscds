cmake_minimum_required(VERSION 2.6)

project(gntf)

set(SRCS chrfmt.cpp gntf.cpp)
set(HEADERS chrfmt.h gntf.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(gntf ${SRCS} ${HEADERS})
target_link_libraries(gntf mscdsa)

add_executable(t_gntf gntf_test.cpp)
target_link_libraries(t_gntf gntf)
set_target_properties(t_gntf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})