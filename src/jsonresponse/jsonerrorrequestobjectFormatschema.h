#pragma once
#include <boost/describe/class.hpp>
#include <string>

template <typename T> struct JsonErrorRequestObjectFormatSchema {
  std::string instruction =
      "the formate of the request should be in the form below";
  T object;
  BOOST_DESCRIBE_CLASS(JsonErrorRequestObjectFormatSchema, (),
                       (instruction, object), (), ());
};
