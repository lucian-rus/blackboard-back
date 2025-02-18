cmake_minimum_required(VERSION 3.0)
project(backend)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY output)

add_subdirectory(components)

add_executable(${PROJECT_NAME} src/main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
# define target platform for the project
add_compile_definitions(_LINUXPLATFORM)
# add_compile_definitions(_LOG_ENABLED)
# define port to which the connection will be linked
add_compile_definitions(_APK_CONN=16080)
add_compile_definitions(_APP_CONN=16081)

target_link_libraries(${PROJECT_NAME}
    logger.lib
    socket.lib
)
