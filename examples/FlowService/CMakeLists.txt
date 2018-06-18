include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(flow_server flow_server.cc ${src_list})
add_executable(flow_client flow_client.cc ${src_list})