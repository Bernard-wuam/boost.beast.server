#pragma once
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <optional>
#include <string>

struct GetPostSchema {
  std::string title;
  std::string content;
  boost::mysql::datetime created_at;
  boost::mysql::datetime last_updated;
  std::string public_id;
  std::string username;
  std::optional<std::string> profile_picture;
};
BOOST_DESCRIBE_STRUCT(GetPostSchema, (),
                      (title, content, created_at, last_updated, public_id,
                       username, profile_picture));