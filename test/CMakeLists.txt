include_directories(${PROJECT_SOURCE_DIR}/src)

#file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(memory_pool_test memory_pool_test.cc ${PROJECT_SOURCE_DIR}/src/memory_pool.cc)
add_executable(buffer_test buffer_test.cc ${PROJECT_SOURCE_DIR}/src/memory_pool.cc ${PROJECT_SOURCE_DIR}/src/buffer.cc)
add_executable(string_performance_test string_performance_test.cc)
add_executable(block_queue_test block_queue_test.cc)