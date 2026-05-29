#pragma once
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <optional>
#include <string>

struct SendUserSchema {
  std::string username;
  std::string email;
  std::string public_id;
  std::optional<boost::mysql::datetime> created_at;
};
BOOST_DESCRIBE_STRUCT(SendUserSchema, (),
                      (username, email, public_id, created_at));