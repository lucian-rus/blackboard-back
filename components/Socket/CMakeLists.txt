add_library(socket.lib STATIC
# sources
  src/server.cpp
# includes
  include/server.h
)

set_target_properties(socket.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(socket.lib PUBLIC include)

target_link_libraries(socket.lib
    logger.lib
)
