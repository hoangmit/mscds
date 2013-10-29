cmake_minimum_required(VERSION 2.6)

project(cwig)

set(SRCS chrfmt.cpp cwig.cpp stringarr.cpp
RLSum6.cpp 
SampledSum.h
rank_vals.h
poly_vals.cpp nintv.cpp)

set(HEADERS chrfmt.h cwig.h stringarr.h 
RLSum6.h 
SampledSum.cpp
rank_vals.cpp
rlsum_int.h poly_vals.h nintv.h)

#add_executable(bedgraph2gnt bedgraph2gnt.cpp ${SRCS} ${HEADERS})
#target_link_libraries(bedgraph2gnt mscdsa)

add_library(cwig ${SRCS} ${HEADERS})
target_link_libraries(cwig mscdsa)

add_executable(t_cwig cwig_test.cpp)
target_link_libraries(t_cwig cwig utils)
set_target_properties(t_cwig PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_rlsum rlsum_test.cpp)
target_link_libraries(t_rlsum intarray cwig ${COMMON_LIBS})
set_target_properties(t_rlsum PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(t_val_arr val_arr_test.cpp)
target_link_libraries(t_val_arr intarray  cwig ${COMMON_LIBS})
set_target_properties(t_val_arr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(tbm_cwig benchmark_cwig.cpp)
target_link_libraries(tbm_cwig cwig)
set_target_properties(tbm_cwig PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})