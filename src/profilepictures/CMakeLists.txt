
add_library(ProfilePictures INTERFACE)
target_sources(
    ProfilePictures
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Profilepicturesroute.h
)
target_link_libraries(ProfilePictures INTERFACE Boost::beast)