add_library(logger.lib STATIC
# sources

# includes
  include/logger.h
)

set_target_properties(logger.lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(logger.lib PUBLIC include)
