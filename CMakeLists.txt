cmake_minimum_required(VERSION 3.10.0)
project(server VERSION 0.1.0 LANGUAGES CXX)

# Set the global C++ standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(boost_beast CONFIG REQUIRED)
find_package(boost_url CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS log)
find_package(Boost REQUIRED COMPONENTS mysql)
find_package(Boost REQUIRED COMPONENTS json)
find_package(Boost REQUIRED COMPONENTS charconv)
find_package(Boost REQUIRED COMPONENTS mysql)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS uuid)
pkg_check_modules(LIBSODIUM REQUIRED libsodium)
find_library(BCRYPT_LIBRARY NAMES bcrypt)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_path(PICOJSON_INCLUDE_DIRS "picojson/picojson.h")
find_package(boost_url CONFIG REQUIRED)

add_executable(server main.cpp)

target_include_directories(server PRIVATE ${SODIUM_INCLUDE_DIRS})
add_subdirectory(src)
target_link_libraries(server PRIVATE Server Boost::beast
    Boost::url Boost::log Boost::mysql Boost::json
    Boost::charconv OpenSSL::Crypto OpenSSL::SSL
    Boost::mysql Threads::Threads ${LIBSODIUM_LIBRARIES}
    AppErrorCode)

target_compile_definitions(server PUBLIC CURRENTPATH="${CMAKE_CURRENT_LIST_DIR}"
    REFRESHTOKENEXPIRATIONTIME=${refreshTokenExpirationTime}
    ACCESSTOKENEXPIRATIONTIME=${accessTokenExpirationTime}
    SECRETEKEY="${secretKey}"
)
