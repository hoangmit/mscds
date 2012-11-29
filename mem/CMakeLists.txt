cmake_minimum_required(VERSION 2.6)

project(mem)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


set(SRCS filearchive.cpp fmaparchive.cpp)
set(HEADERS filearchive.h fmaparchive.h)

add_library(mem ${SRCS} ${HEADERS})

add_executable(t_mem filemaptest.cpp)
target_link_libraries(t_mem mem)
set_target_properties(t_mem PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})