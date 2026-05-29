#pragma once
#include "users/schemas/sendloginuserschema.h"
#include <boost/describe/class.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/datetime.hpp>
#include <string>

struct LoginSuccessFullSchema {
  SendLoginUserSchema userInformation;
  std::string accessToken;
  std::string refreshToken;
};
BOOST_DESCRIBE_STRUCT(LoginSuccessFullSchema, (),
                      (userInformation, accessToken, refreshToken));
