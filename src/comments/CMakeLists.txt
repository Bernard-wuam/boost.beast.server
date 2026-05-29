add_library(Comments INTERFACE)

target_sources(
    Comments
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getpostcommentroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getcommentroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/createpostcommentroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/deletecommentroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/updatecommentroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getpostcommentdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/deletecommentdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/updatecommentdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/createpostcommentdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getcommentdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/createpostcommentschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/updatecommentschema.h
)
target_link_libraries(Comments INTERFACE Boost::json Boost::beast Boost::mysql AppErrorCode)