cmake_minimum_required(VERSION 3.0)
project(backend)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)

# generic defines
add_compile_definitions(_LINUXPLATFORM)
add_compile_definitions(_LOG_ENABLED)

# add subdirectories to be added to cmpilation
add_subdirectory(components)

add_executable(${PROJECT_NAME} src/main.cpp)
# main application defines
add_compile_definitions(_LOG_DRIVER_TAG="Application       | ")
add_compile_definitions(_ENABLE_TX_APP)
add_compile_definitions(_ENABLE_RX_APP)
add_compile_definitions(_CLIENT_CONN_LIMIT=2)
add_compile_definitions(_SOCKET_NUMBER=8080)

# set properties here
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# link additional libraries needed
target_link_libraries(${PROJECT_NAME}
    TCPConnection.lib
    logger.lib
)
