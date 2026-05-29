#pragma once
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <string>

struct SendPostSchema {
  std::string title;
  std::string content;
  boost::mysql::datetime created_at;
  boost::mysql::datetime last_updated;
};
BOOST_DESCRIBE_STRUCT(SendPostSchema, (),
                      (title, content, created_at, last_updated));