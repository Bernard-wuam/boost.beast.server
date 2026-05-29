#pragma once
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <string>

struct LoginUserSchema {
  std::string emailOrUsername = "johndoe or johndoe@yahoo.com";
  std::string password = "1234@#jhkk";
};
BOOST_DESCRIBE_STRUCT(LoginUserSchema, (),
                      ( emailOrUsername,password));