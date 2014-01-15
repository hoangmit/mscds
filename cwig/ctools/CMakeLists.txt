
cmake_minimum_required(VERSION 2.6)
project(cwig_tools)


###################

set(LIBS cwig mscdsa)

add_executable(bedgraph2cwig bedgraph2cwig.cpp)
target_link_libraries(bedgraph2cwig ${LIBS})

add_executable(cwigSummary cwig_summary.cpp)
target_link_libraries(cwigSummary ${LIBS})

add_executable(cwigInfo cwig_info.cpp)
target_link_libraries(cwigInfo  ${LIBS})

add_executable(cwig2bedgraph cwig2bedgraph.cpp)
target_link_libraries(cwig2bedgraph ${LIBS})

set_property(TARGET bedgraph2cwig PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwigSummary PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwigInfo PROPERTY FOLDER "Apps/cTools")
set_property(TARGET cwig2bedgraph PROPERTY FOLDER "Apps/cTools")



