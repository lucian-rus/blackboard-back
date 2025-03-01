add_library(TCPMessage.lib STATIC
# sources
  src/TCPMessage.cpp
# includes
  include/TCPMessage.h
)

set_target_properties(TCPMessage.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(TCPMessage.lib PUBLIC include)

target_link_libraries(TCPMessage.lib
    logger.lib
)
