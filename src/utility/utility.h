#pragma once

#include "apperror/apperror.h"
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/parse.hpp>
#include <boost/mysql/datetime.hpp>
#include <expected>
#include <system_error>

namespace Utility {
template <typename T>
inline std::expected<T, std::error_code> convertBodyToObject(
    boost::beast::http::request<boost::beast::http::string_body> &request) {
  boost::beast::error_code ec;
  auto obj = boost::json::parse(request.body(), ec);
  if (ec)
    return std::unexpected(
        makeErrorCode(AppError::Convert_Body_To_Object_Failed));
  auto tryVal = boost::json::try_value_to<T>(obj);
  if (tryVal.has_error()) {
    return std::unexpected(
        makeErrorCode(AppError::Convert_Body_To_Object_Failed));
  }
  return tryVal.value();
}

inline std::string formatTime(const boost::mysql::datetime &datetime) {
  auto time = datetime.as_time_point();
  auto timeString = std::format("{:%Y-%m-%d %H:%M:%S}", time);

  auto findDotPos = timeString.rfind(".");
  if (findDotPos != timeString.npos) {
    timeString = timeString.substr(0, findDotPos);
  }
  return timeString;
}
inline std::string getImageExtention(const std::string &target) {
  auto extentionIndex = target.rfind(".");
  auto extention = target.substr(extentionIndex + 1, target.size());
  std::string imageExtention = "image/";
  imageExtention.append(extention);
  return imageExtention;
}
} // namespace Utility