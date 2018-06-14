include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(transfer_server transfer_server.cc ${src_list})
add_executable(transfer_client transfer_client.cc ${src_list})