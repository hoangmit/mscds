
cmake_minimum_required(VERSION 2.6)

project(tree)

set(SRCS ReadTree.cpp BP_bits.cpp RMQ_sct.cpp)
set(HEADERS CartesianTree.h LCA.h ReadTree.h RMQ_table.h tree.h BP_bits.h RMQ_sct.h)

add_library(tree ${SRCS} ${HEADERS})
target_link_libraries(tree bitarray)

add_executable(t_rmq rmq_test.cpp)
target_link_libraries(t_rmq tree)
set_target_properties(t_rmq PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})

add_executable(t_bptree bptree_test.cpp)
target_link_libraries(t_bptree tree)
set_target_properties(t_bptree PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})