
add_library(Security INTERFACE)
target_sources(
    Security
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/security.h
)
target_link_libraries(Security INTERFACE OpenSSL::Crypto 
OpenSSL::SS Boost::charconv ${PICOJSON_INCLUDE_DIRS} jwt-cpp::jwt-cpp)