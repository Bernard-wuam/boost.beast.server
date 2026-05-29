add_library(Users INTERFACE)
target_sources(
    Users
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/createuserschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/loginuserschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/sendloginuserschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/loginsuccessfulschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/refreshuserschema.h
     ${CMAKE_CURRENT_SOURCE_DIR}/schemas/sendrefreshandaccesstokenschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/createuserroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/loginuserroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/refreshuserroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/createuserdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/loginuserdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/refreshuserdb.h
)
target_link_libraries(Users INTERFACE Boost::json Boost::beast Boost::mysql Boost::uuid AppErrorCode Utility)
