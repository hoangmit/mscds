
cmake_minimum_required(VERSION 2.6)
project(cwig_tools)


###################

FIND_PACKAGE(Boost 1.50 COMPONENTS program_options REQUIRED )

set(LIBS cwig mscdsa ${Boost_LIBRARIES})

add_executable(bedgraph2cwig bedgraph2cwig.cpp)
target_link_libraries(bedgraph2cwig ${LIBS})
strip_target(bedgraph2cwig)

add_executable(cwigSummary cwig_summary.cpp)
target_link_libraries(cwigSummary ${LIBS})
strip_target(cwigSummary)

add_executable(cwigSummaryBatch cwig_summary_batch.cpp)
target_link_libraries(cwigSummaryBatch ${LIBS})
strip_target(cwigSummaryBatch)

add_executable(cwigInfo cwig_info.cpp)
target_link_libraries(cwigInfo  ${LIBS})
strip_target(cwigInfo)

add_executable(cwig2bedgraph cwig2bedgraph.cpp)
target_link_libraries(cwig2bedgraph ${LIBS})
strip_target(cwig2bedgraph)

add_executable(cwig_benchmark cwig_benchmark.cpp)
target_link_libraries(cwig_benchmark ${LIBS})
strip_target(cwig_benchmark)


set_property(TARGET bedgraph2cwig PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwigSummary PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwigSummaryBatch PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwigInfo PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwig2bedgraph PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwig_benchmark PROPERTY FOLDER "Apps/cTools")


if (USE_SSL)
    find_package(OpenSSL)
endif()

if (OPENSSL_FOUND AND USE_SSL)
    add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
    include_directories(${OPENSSL_INCLUDE_DIR})
	set(SERVER_LIBS ${LIBS} ${OPENSSL_LIBRARIES} cppnetlib-uri cppnetlib-server-parsers )
else()
	set(SERVER_LIBS ${LIBS} cppnetlib-uri cppnetlib-server-parsers )
endif()

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${NETLIB_INCLUDE_DIRS})

add_executable(cwig_server url_parser.cpp url_parser.h cwig_server.cpp)
TARGET_LINK_LIBRARIES(cwig_server ${Boost_LIBRARIES} ${SERVER_LIBS})
if (UNIX)
    TARGET_LINK_LIBRARIES(cwig_server -lrt)
endif()
strip_target(cwig_server)
set_property(TARGET cwig_server PROPERTY FOLDER "Apps/cTools")

####------------------------------------########
## Try to find JKENT_LIB for bigWig2cwig      ##



set(BUILD_BIGWIG2CWIG OFF)
set(JKENT_LIB_INCLUDE_DIR "~/tools/kent/src/inc/")
set(JKENT_LIB_PATH "~/tools/kent/src/lib/x86_64/")

IF (BUILD_BIGWIG2CWIG)
	find_library(JKENT_LIB NAMES "jkweb" 
        PATHS ${JKENT_LIB_PATH} )
ENDIF()
IF (BUILD_BIGWIG2CWIG AND JKENT_LIB)
    find_package(ZLIB REQUIRED)
    find_package(Threads REQUIRED)
    find_package(OpenSSL REQUIRED)

    MESSAGE(STATUS "Linking with :" ${JKENT_LIB})
	include_directories(${JKENT_LIB_INCLUDE_DIR})
	add_executable(bigWig2cwig bigWig2cwig.cpp bw_scan.cpp bw_scan.h)
	target_link_libraries(bigWig2cwig ${LIBS} ${JKENT_LIB} ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} -lssl -lcrypto)
ENDIF()


