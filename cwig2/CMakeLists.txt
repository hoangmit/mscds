cmake_minimum_required(VERSION 2.6)

project(cwig2)

include_directories(${Boost_INCLUDE_DIRS})

set(SRCS 
fused_intval.cpp
intv/nintv_fuse.cpp
)

set(HEADERS
fused_intval.h
intv/nintv_fuse.h
float_mono_array.h
float_int_map.h
)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(cwig2 ${SRCS} ${HEADERS})
target_link_libraries(cwig2 mscdsa remote_file fusionarray cwig)
set_property(TARGET cwig2 PROPERTY FOLDER "Apps")

#add_test_files(intv/intv_tests.cpp)
#intv/intv_rand_gen.hpp
 
#add_executable(t_cwig2  intv/intv_tests.cpp)
#target_link_libraries(t_cwig cwig utils)
#set_target_properties(t_cwig PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_cwig PROPERTY FOLDER "Tests")


add_executable(t_cwig_fuse intv/nintv_fuse_test.cpp)
target_link_libraries(t_cwig_fuse cwig cwig2 utils)
set_target_properties(t_cwig_fuse PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET t_cwig_fuse PROPERTY FOLDER "Tests")

