#pragma once
#include <boost/describe/class.hpp>
#include <string>
struct CategorySchema {
  std::string music = "music";
  std::string life = "life";
  std::string sport = "sport";
};
BOOST_DESCRIBE_STRUCT(CategorySchema, (), (music, life, sport));