cmake_minimum_required(VERSION 2.6)

project(cwig2)

include_directories(${Boost_INCLUDE_DIRS})

set(SRCS 
intv/nintv_fuse.cpp
float_int_map.cpp
fusedstorage.cpp
fusedstorage2.cpp
)

set(HEADERS
fused_intval2.h
fusedstorage.h
fusedstorage2.h
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

add_test_exec(t_cwig_fuse FILES test_float_int.cpp intv/nintv_fuse_test.cpp test_intval.cpp LIBS cwig2 utils)

