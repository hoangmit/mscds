cmake_minimum_required(VERSION 2.6)

project(sdarray)

set(SRCS sdarray.cpp sdarray_sml.cpp codearray.cpp)
set(HEADERS sdarray.h sdarray_sml.h codearray.h)

add_library(sdarray ${SRCS} ${HEADERS})
target_link_libraries(sdarray bitarray codec)
#add_sources(mscdsa ${SRCS} ${HEADERS})

add_executable(t_sda sda_test.cpp)
target_link_libraries(t_sda sdarray ${COMMON_LIBS})
set_target_properties(t_sda PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_codearr codearr_test.cpp)
target_link_libraries(t_codearr sdarray ${COMMON_LIBS})
set_target_properties(t_codearr PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
