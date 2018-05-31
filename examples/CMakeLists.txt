include_directories(${PROJECT_SOURCE_DIR}/src)

#aux_source_directory(${PROJECT_SOURCE_DIR}/src/*.cc src_list)

add_executable(echo_server echo_server.cc ${PROJECT_SOURCE_DIR}/src/stream.cc ${PROJECT_SOURCE_DIR}/src/event_loop.cc)
add_executable(echo_client echo_client.cc ${PROJECT_SOURCE_DIR}/src/stream.cc ${PROJECT_SOURCE_DIR}/src/event_loop.cc)
