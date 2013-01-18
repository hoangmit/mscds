cmake_minimum_required(VERSION 2.6)

project(intarray)

set(SRCS sdarray.cpp sdarray_sml.cpp codearray.cpp)
set(HEADERS sdarray.h sdarray_sml.h codearray.h)

add_library(intarray ${SRCS} ${HEADERS})
target_link_libraries(intarray bitarray codec)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_executable(t_sda sda_test.cpp)
target_link_libraries(t_sda intarray ${COMMON_LIBS})
set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_codearr codearr_test.cpp)
target_link_libraries(t_codearr intarray ${COMMON_LIBS})
set_target_properties(t_codearr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
