
cmake_minimum_required(VERSION 2.6)

project(tree)

set(SRCS ReadTree.cpp BP_bits.cpp RMQ_sct.cpp RMQ_table.cpp RMQ_index_table.cpp RMQ_pm1.cpp)
set(HEADERS CartesianTree.h LCA.h ReadTree.h RMQ_table.h RMQ_index_table.h tree.h BP_bits.h RMQ_sct.h RMQ_pm1.h RMQ_lca.h)

add_library(tree ${SRCS} ${HEADERS})
target_link_libraries(tree bitarray intarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})



add_test_files(rmq_test.cpp)
#add_executable(t_rmq rmq_test.cpp)
#target_link_libraries(t_rmq tree utils)
#set_target_properties(t_rmq PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_rmq_test2 PROPERTY FOLDER "Tests")

add_test_files(bptree_test.cpp)
#add_executable(t_bptree bptree_test.cpp)
#target_link_libraries(t_bptree tree utils ${COMMON_LIBS})
#set_target_properties(t_bptree PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_rmq_test2 PROPERTY FOLDER "Tests")


add_test_files(rmq_test2.cpp)
#add_executable(t_rmq_test2 rmq_test2.cpp)
#target_link_libraries(t_rmq_test2 tree utils ${COMMON_LIBS} ${HAYAI_LIBRARY})
#set_target_properties(t_rmq_test2 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
#set_property(TARGET t_rmq_test2 PROPERTY FOLDER "Tests")

add_benchmark_files(rmq_benchmark.cpp)