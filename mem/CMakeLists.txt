cmake_minimum_required(VERSION 2.6)

project(mem)

#find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


set(SRCS file_archive1.cpp fmap_archive1.cpp impl/file_marker.cpp info_archive.cpp
file_archive2.cpp
fmap_archive2.cpp
)
set(HEADERS file_archive1.h fmap_archive1.h impl/file_marker.h info_archive.h
local_mem.h
../framework/archive.h ../framework/mem_models.h
file_archive2.h
fmap_archive2.h
save_load_test.h
shortcuts.h
)
add_library(mem ${SRCS} ${HEADERS})
target_link_libraries(mem utils)
#add_sources(mscdsa ${SRCS} ${HEADERS})


add_test_files(filemap_test.cpp)

add_test_files(mem_test.cpp)

