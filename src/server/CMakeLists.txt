
add_library(Server INTERFACE)

target_sources(
    Server
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/server.h
)
