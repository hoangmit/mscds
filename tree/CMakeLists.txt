
cmake_minimum_required(VERSION 2.6)

project(tree)

set(SRCS ReadTree.cpp BP_bits.cpp RMQ_sct.cpp RMQ_table.cpp RMQ_index_table.cpp RMQ_pm1.cpp)
set(HEADERS CartesianTree.h LCA.h ReadTree.h RMQ_table.h RMQ_index_table.h tree.h BP_bits.h RMQ_sct.h RMQ_pm1.h RMQ_lca.h)

add_library(tree ${SRCS} ${HEADERS})
target_link_libraries(tree bitarray intarray)
#add_sources(mscdsa ${SRCS} ${HEADERS})



add_test_files(rmq_test.cpp)


add_test_files(bptree_test.cpp)



add_test_files(rmq_test2.cpp)


add_benchmark_files(rmq_benchmark.cpp)