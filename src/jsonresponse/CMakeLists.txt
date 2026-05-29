add_library(JsonResponse INTERFACE)

target_sources(
    JsonResponse
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonresponse.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonerrorrequestobjectformatschema.h
)
target_link_libraries(JsonResponse INTERFACE Boost::json Boost::beast AppErrorCode)