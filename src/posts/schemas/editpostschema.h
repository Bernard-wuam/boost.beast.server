#pragma once
#include <boost/describe/class.hpp>
#include <string>

struct EditPostSchema {

  std::string title = "the king of flies";
  std::string content =
      "The book was, awsome, the writter has, a good story telling technique.";
  std::string category;
};
BOOST_DESCRIBE_STRUCT(EditPostSchema, (),
                      ( title, content,category));