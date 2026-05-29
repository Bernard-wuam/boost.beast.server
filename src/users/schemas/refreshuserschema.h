#pragma once
#include <boost/describe/class.hpp>
#include <string>
struct RefreshUserSchema{
    std::string refreshToken = "22842978-4647-11f1-8f4c-00155dfeee3d";
};
BOOST_DESCRIBE_STRUCT(RefreshUserSchema,(),(refreshToken));