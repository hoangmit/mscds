cmake_minimum_required(VERSION 2.6)

project(mem)

#find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


set(SRCS file_archive.cpp fmap_archive.cpp impl/file_marker.cpp info_archive.cpp)
set(HEADERS file_archive.h fmap_archive.h impl/file_marker.h info_archive.h local_mem.h
../framework/archive.h ../framework/mem_models.h)
add_library(mem ${SRCS} ${HEADERS})
target_link_libraries(mem utils)
#add_sources(mscdsa ${SRCS} ${HEADERS})



add_test_files(filemap_test.cpp)
#add_executable(t_mem filemaptest.cpp)
#target_link_libraries(t_mem mem)
#set_target_properties(t_mem PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})


add_executable(t_mem2 mem_test.cpp)
target_link_libraries(t_mem2 mem)
set_target_properties(t_mem2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
set_property(TARGET t_mem2 PROPERTY FOLDER "Tests")