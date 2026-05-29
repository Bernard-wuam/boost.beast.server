#pragma once
#include <boost/describe/class.hpp>
#include <string>

struct JwtUserInfoSchema {
  std::string public_id;
  std::string role;
};
BOOST_DESCRIBE_STRUCT(JwtUserInfoSchema, (), (public_id, role));