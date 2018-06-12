include_directories(${PROJECT_SOURCE_DIR}/src)

#file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

set(src_dir ${PROJECT_SOURCE_DIR}/src/)

add_executable(memory_pool_test 
    memory_pool_test.cc 
    ${src_dir}/memory_pool.cc 
    ${src_dir}/log.cc
    ${src_dir}/log_stream.cc
    ${src_dir}/fix_buffer.cc)
add_executable(buffer_test 
    buffer_test.cc 
    ${src_dir}/memory_pool.cc 
    ${src_dir}/buffer.cc 
    ${src_dir}/log.cc
    ${src_dir}/log_stream.cc
    ${src_dir}/fix_buffer.cc)
add_executable(string_performance_test string_performance_test.cc)
add_executable(block_queue_test 
    block_queue_test.cc 
    ${src_dir}/log.cc
    ${src_dir}/log_stream.cc
    ${src_dir}/fix_buffer.cc)
add_executable(log_test 
    log_test.cc ${src_dir}/log.cc 
    ${src_dir}/log_stream.cc
    ${src_dir}/fix_buffer.cc)