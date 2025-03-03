add_library(NetworkMessage.lib STATIC
# sources
  src/NetworkMessage.cpp
# includes
  include/NetworkMessage.h
)

add_compile_definitions(_LOG_DRIVER_TAG="NetworkMessage    | ")
set_target_properties(NetworkMessage.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(NetworkMessage.lib PUBLIC include)

target_link_libraries(NetworkMessage.lib
    logger.lib
)
