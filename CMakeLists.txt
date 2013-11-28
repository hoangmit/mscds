cmake_minimum_required(VERSION 2.6)

project(mscds)
set(VERSION 0.1.1)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE RELEASE CACHE STRING
	  "Choose the type of build, options are: None Debug Release."
	  FORCE)
endif (NOT CMAKE_BUILD_TYPE)

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(cmake/function.cmake)
include(cmake/MergeStaticLibraries.cmake)
fix_compiler_flags() #defined inside function.cmake

#setup directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
   CACHE PATH "Single Directory for all Executables.")

set(TEST_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests
	CACHE PATH "Directory for testings executables")
set(MODULE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/modules
	CACHE PATH "Directory for python modules")

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
	
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC -O3 -g")
else()
	if(MSVC)
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
		add_definitions(/wd4244 /wd4018)
		add_definitions(/D_VARIADIC_MAX=10)
		#add_definitions(/arch:SSE)
	endif()
endif()
#  .exe = runtime
#  .dll = runtime
#  .lib = archive  (either import lib or static lib)
#  .a   = archive
#  .so  = library

#---External packcage------------------------------------------------------------------------------
find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/gtest) #${PROJECT_BINARY_DIR}/ext_libs/gtest
set(GTEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/gtest/include)
set(GTEST_LIBRARY gtest)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/snappy)
set(SNAPPY_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/snappy)
set(SNAPPY_LIBRARY snappy)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/zlib)
set(ZLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/zlib)
set(ZLIB_LIBRARY zlibstatic)

include_directories(${GTEST_INCLUDE_DIRS})

#add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/Celero)
#set(CELERO_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/Celero/include)
#set(CELERO_LIBRARY celero)

#add_subdirectory(${PROJECT_SOURCE_DIR}/ext_libs/hayai)
#set(HAYAI_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/ext_libs/hayai)
#set(HAYAI_LIBRARY hayai_main)

#---Interal Utilities-----------------------------------------------------------------------------
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/runtests.py ${TEST_OUTPUT_DIRECTORY}/runtests.py COPYONLY)
	
include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}/framework)

add_subdirectory(${PROJECT_SOURCE_DIR}/mem)
add_subdirectory(${PROJECT_SOURCE_DIR}/utils)

set(COMMON_LIBS mem utils)

########################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/extcodec)


add_subdirectory(${PROJECT_SOURCE_DIR}/bitarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/codec)
add_subdirectory(${PROJECT_SOURCE_DIR}/intarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/wavarray)
add_subdirectory(${PROJECT_SOURCE_DIR}/tree)

set(CORE_LIBS utils mem bitarray codec intarray wavarray tree)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
   CACHE PATH "Single Directory for all Libraries")  
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
   CACHE PATH "Single Directory for all static libraries.")
#set_target_properties(mscdsa PROPERTIES
#         LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
#set_target_properties(mscdsa PROPERTIES
#	ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_BINARY_DIR}/lib)


MERGE_STATIC_LIBRARIES(mscdsa "${CORE_LIBS}")
add_subdirectory(${PROJECT_SOURCE_DIR}/unittests)

MERGE_STATIC_LIBRARIES(extcodeca "extcodec;${SNAPPY_LIBRARY};${ZLIB_LIBRARY}")
	
#get_property(mscdsa_location TARGET mscdsa PROPERTY LOCATION)
#message(" Location =" ${mscdsa_location})
#get_property(mscdsa_SRCS GLOBAL PROPERTY mscdsa_SRCS)
#add_library(mscdsa STATIC ${mscdsa_SRCS})
#add_library(mscdsa STATIC common/mscdsa.h common/mscdsa.cpp)

########################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/export_lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/cwig)
add_subdirectory(${PROJECT_SOURCE_DIR}/cbed)
#add_subdirectory(${PROJECT_SOURCE_DIR}/mfm)
#add_subdirectory(${PROJECT_SOURCE_DIR}/gtags)


