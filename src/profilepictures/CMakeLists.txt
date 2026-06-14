
add_library(ProfilePictures INTERFACE)
target_sources(
    ProfilePictures
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/profilepictureuploadroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getprofilepictureroute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/db/updateprofilepicturedb.h
)
target_link_libraries(ProfilePictures INTERFACE Boost::beast)