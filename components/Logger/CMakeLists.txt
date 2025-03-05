add_library(Logger.lib STATIC
# sources

# includes
  include/Logger.h
)

set_target_properties(Logger.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Logger.lib PUBLIC include)
