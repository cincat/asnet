include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(echo_server echo_server.cc ${src_list})
add_executable(echo_client echo_client.cc ${src_list})
