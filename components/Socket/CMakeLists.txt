add_library(socket.lib STATIC
# sources

# includes
  include/server.h
)

set_target_properties(socket.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(socket.lib PUBLIC include)
