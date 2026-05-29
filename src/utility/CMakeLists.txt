add_library(Utility INTERFACE)

target_sources(
    Utility
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/utility.h
)
target_link_libraries(Utility INTERFACE Boost::json Boost::beast Boost::mysql AppErrorCode)
