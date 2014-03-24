cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS sdarray.cpp sdarray_sml.cpp deltaarray.cpp huffarray.cpp 
gamma_arr.cpp
remap_dt.cpp
blkgroup_array.cpp
sdarray_block.cpp
)

set(HEADERS sdarray.h sdarray_sml.h deltaarray.h intarray.h 
huffarray.h gamma_arr.h
remap_dt.h
blkarray.hpp diffarray.hpp
transform_utils.hpp
contextfree_models.hpp
blkgroup_array.h
sdarray_block.h
)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray codec utils)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_executable(blkgroup_array_test blkgroup_array_test.cpp)
target_link_libraries(blkgroup_array_test intarray utils ${COMMON_LIBS})
set_target_properties(blkgroup_array_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET blkgroup_array_test PROPERTY FOLDER "Tests")

add_test_files(sda_test.cpp)
#add_executable(t_sda sda_test.cpp)
#target_link_libraries(t_sda intarray ${COMMON_LIBS})
#set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_test_files(codearr_test.cpp)
#add_executable(t_codearr codearr_test.cpp)
#target_link_libraries(t_codearr intarray ${COMMON_LIBS})
#set_target_properties(t_codearr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_test_files(intarr_test.cpp)
#add_executable(t_intarray intarr_test.cpp)
#target_link_libraries(t_intarray intarray ${COMMON_LIBS})
#set_target_properties(t_intarray PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
