#pragma once
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <optional>
#include <string>

struct SendLoginUserSchema {
  std::string public_id;
  std::string username;
  std::string role;
  std::optional<std::string> profile_picture;
};
BOOST_DESCRIBE_STRUCT(SendLoginUserSchema, (),
                      (public_id, username, role, profile_picture));