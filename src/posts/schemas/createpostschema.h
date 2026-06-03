#pragma once
#include <boost/describe/class.hpp>
#include <string>

struct CreatePostSchema {
  std::string public_id = "a51a1dd3-4860-11f1-8c5f-00155dfeed4c";
  std::string title = "the king of flies";
  std::string content =
      "The book was, awsome, the writter has, a good story telling technique.";
  std::string image_url = "http://localhost:5333/api/posts/image/"
                          "0f552ec2-cac9-4443-bf54-5a20868fcf34.jpg";
  std::string category;
};
BOOST_DESCRIBE_STRUCT(CreatePostSchema, (),
                      (public_id, title, content, image_url, category));