cmake_minimum_required(VERSION 2.6)

project(cwig)

include_directories(${Boost_INCLUDE_DIRS})

set(SRCS chrfmt.cpp cwig.cpp
RLSum6.cpp 
SampledSum.cpp
rank_vals.cpp
poly_vals.cpp
intv/nintv.cpp
intv/nintv_fuse.cpp
)

set(HEADERS chrfmt.h cwig.h 
RLSum6.h 
SampledSum.h
rank_vals.h
rlsum_int.h poly_vals.h
intv/nintv.h
intv/nintv_fuse.h
)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(cwig ${SRCS} ${HEADERS})
target_link_libraries(cwig mscdsa remote_file)
set_property(TARGET cwig PROPERTY FOLDER "Apps")

add_executable(t_cwig cwig_test.cpp rlsum_test.cpp val_arr_test.cpp
intv/intv_tests.cpp intv/intv_rand_gen.hpp)
target_link_libraries(t_cwig cwig utils)
set_target_properties(t_cwig PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET t_cwig PROPERTY FOLDER "Tests")

add_executable(tbm_cwig benchmark_cwig.cpp)
target_link_libraries(tbm_cwig cwig)
set_target_properties(tbm_cwig PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET tbm_cwig PROPERTY FOLDER "Tests")


add_subdirectory(${PROJECT_SOURCE_DIR}/ctools/)
