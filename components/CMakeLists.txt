### define socket library
add_library(socket.lib STATIC
    Socket/include/server.h
)

set_target_properties(socket.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(socket.lib PUBLIC Socket/include)

### define logger library 
add_library(logger.lib STATIC
    Logger/include/logger.h
)

set_target_properties(logger.libPROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(logger.lib PUBLIC Logger/include)