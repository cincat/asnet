include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB src_list ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(pingpong_server pingpong_server.cc ${src_list})
add_executable(pingpong_client pingpong_client.cc ${src_list})