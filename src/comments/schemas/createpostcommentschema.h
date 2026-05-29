#pragma once
#include <boost/describe/class.hpp>
#include <optional>
#include <string>

struct CreatePostCommentSchema {
  std::optional<std::string> reply_to = "the king of flies";
  std::string content =
      "The book was, awsome, the writter has, a good story telling technique.";
};
BOOST_DESCRIBE_STRUCT(CreatePostCommentSchema, (), (reply_to, content));