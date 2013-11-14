cmake_minimum_required(VERSION 2.6)

project(mscds_tests)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(mscds_tests test_main.cpp ${TEST_FILES})
target_link_libraries(mscds_tests mscdsa utils)
set_target_properties(mscds_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})



