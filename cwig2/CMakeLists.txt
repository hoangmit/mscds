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
 

add_test_exec(t_float_int FILES test_float_int.cpp LIBS cwig2 utils)

add_test_exec(t_cwig_fuse FILES intv/nintv_fuse_test.cpp LIBS cwig cwig2 utils)

