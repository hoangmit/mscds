cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS sdarray.cpp sdarray_sml.cpp deltaarray.cpp huffarray.cpp 
gamma_arr.cpp
remap_dt.cpp
)

set(HEADERS sdarray.h sdarray_sml.h deltaarray.h intarray.h 
huffarray.h gamma_arr.h
remap_dt.h
blkarray.hpp diffarray.hpp
transform_utils.hpp
contextfree_models.hpp
)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray codec)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_test_files(sda_test.cpp)
#add_executable(t_sda sda_test.cpp)
#target_link_libraries(t_sda intarray ${COMMON_LIBS})
#set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_test_files(codearr_test.cpp)
#add_executable(t_codearr codearr_test.cpp)
#target_link_libraries(t_codearr intarray ${COMMON_LIBS})
#set_target_properties(t_codearr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(t_intarray intarr_test.cpp)
target_link_libraries(t_intarray intarray ${COMMON_LIBS})
set_target_properties(t_intarray PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
