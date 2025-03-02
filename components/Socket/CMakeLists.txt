add_library(TCPConnection.lib STATIC
# sources
  src/TCPConnection.cpp
# includes
  include/TCPConnection.h
)

add_compile_definitions(_LOG_DRIVER_TAG="TCPMessage  | ")
set_target_properties(TCPConnection.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(TCPConnection.lib PUBLIC include)

target_link_libraries(TCPConnection.lib
    logger.lib
)
