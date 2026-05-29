#pragma once
#include <boost/describe/class.hpp>
#include <string>
struct CreateUserSchema{
    std::string username = "johndoe";
    std::string email = "johndoe@yahoo.com";
    std::string password = "1234@#jhkk";
};
BOOST_DESCRIBE_STRUCT(CreateUserSchema,(),(username,email,password));