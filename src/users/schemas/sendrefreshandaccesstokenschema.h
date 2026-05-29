#pragma once
#include <boost/describe/class.hpp>
#include <string>
struct SendRefreshAndAcessTokenSchema {
  std::string accessToken ;
  std::string refreshToken ;
  
};
BOOST_DESCRIBE_STRUCT(SendRefreshAndAcessTokenSchema, (),
                      (accessToken, refreshToken));