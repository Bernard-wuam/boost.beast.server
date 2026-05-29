
add_library(RegexRouter INTERFACE)
target_sources(
    RegexRouter
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/regexrouter.h
)
