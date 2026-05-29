#pragma once
#include <boost/describe/class.hpp>
#include <string>

struct UpdateCommentSchema {
  std::string content =
      "The book was, awsome, the writter has, a good story telling technique.";
};
BOOST_DESCRIBE_STRUCT(UpdateCommentSchema, (), (content));