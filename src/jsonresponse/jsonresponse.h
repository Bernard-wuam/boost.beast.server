#pragma once

#include "apperror/apperror.h"
#include <boost/beast.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_from.hpp>
#include <system_error>

namespace JsonResponse {

inline boost::beast::http::response<boost::beast::http::string_body>
sendJsonBadResponse(int version, bool keepAlive, std::error_code ec) {

  boost::beast::http::response<boost::beast::http::string_body> response;
  response.version(version);
  response.keep_alive(keepAlive);
  response.set(boost::beast::http::field::content_type, "application/json");

  switch (static_cast<AppError>(ec.value())) {
  case AppError::Email_Already_In_Use:
  case AppError::Username_Already_In_use:
  case AppError::Invalid_Credentials:
  case AppError::Invalid_AcessToken:
  case AppError::Invalid_RefreshToken:
  case AppError::Unauthorize_Access: {
    response.result(boost::beast::http::status::unauthorized);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", ec.message()}});
    break;
  }
  case AppError::Reading_Request_Failed:
  case AppError::Convert_Body_To_Object_Failed:
  case AppError::Writing_Response_Failed:
  case AppError::Password_Not_Good:
  case AppError::No_Database_Connection:
  case AppError::Server_Failed:
  case AppError::Invalid_Path_Parameter: {
    response.result(boost::beast::http::status::internal_server_error);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", ec.message()}});
    break;
  }
  case AppError::User_Not_Found:
  case AppError::Post_Not_Found:
  case AppError::Comment_Not_Found:
  case AppError::Category_Not_Found: {
    response.result(boost::beast::http::status::not_found);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", ec.message()}});
    break;
  }
  case AppError::AccessToken_Expired:
  case AppError::RefreshToken_Expired: {
    response.result(
        boost::beast::http::status::network_authentication_required);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", ec.message()}});
    break;
  }
  case AppError::Illegal_post_Operation:
  case AppError::Illegal_Comment_Operation: {
    response.result(boost::beast::http::status::not_acceptable);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", ec.message()}});
    break;
  }
  default: {
    response.result(boost::beast::http::status::unknown);
    response.body() =
        boost::json::serialize(boost::json::value{{"result", "unknown"}});
    break;
  }
  }

  response.prepare_payload();
  return response;
};

template <typename T>
boost::beast::http::response<
    boost::beast::http::string_body> inline sendJsonResponse(int version,
                                                             bool KeepAlive,
                                                             const T &obj) {
  boost::beast::http::response<boost::beast::http::string_body> response(
      boost::beast::http::status::ok, version);
  response.set(boost::beast::http::field::content_type, "application/json");
  response.body() = boost::json::serialize(boost::json::value_from(obj));
  response.keep_alive(KeepAlive);
  response.prepare_payload();
  return response;
}

template <typename T>
boost::beast::http::response<
    boost::beast::http::
        string_body> inline sendJsonBadObjectFormatResponse(int version,
                                                            bool KeepAlive,
                                                            const T &obj) {
  boost::beast::http::response<boost::beast::http::string_body> response(
      boost::beast::http::status::not_acceptable, version);
  response.set(boost::beast::http::field::content_type, "application/json");
  response.body() = boost::json::serialize(boost::json::value_from(obj));
  response.keep_alive(KeepAlive);
  response.prepare_payload();
  return response;
}

}; // namespace JsonResponse