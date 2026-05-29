
add_library(Likes INTERFACE)

target_sources(
    Likes
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/createlikeroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/deletelikeroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/dislikeroute.h
     ${CMAKE_CURRENT_SOURCE_DIR}/routes/deletedislikeroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/createlikedb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/createdislikedb.h
     ${CMAKE_CURRENT_SOURCE_DIR}/db/deletedislikedb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/deletelikedb.h
)