add_library(Post INTERFACE)

target_sources(
    Post
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/postroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/createpostroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getpostroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/deletepostroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getpostbycategoryroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getpostbyuseroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/uploadroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getimageroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/createpostdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/deletepostdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getpostdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getallpostdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getallpostdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getpostbycategorydb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/getpostbyuserdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/createpostschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/getpostschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/sendpostschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/schemas/jwtuserinfoschema.h
)

target_link_directories(Post INTERFACE Boost::url)