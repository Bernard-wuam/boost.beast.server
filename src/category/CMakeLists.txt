
add_library(Category INTERFACE)
target_sources(
    Category
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/categoryschema.h
    ${CMAKE_CURRENT_SOURCE_DIR}/routes/getcategoryschema.h
      ${CMAKE_CURRENT_SOURCE_DIR}/db/getcategoryschema.h
)