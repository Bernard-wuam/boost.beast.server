
#pragma once
#include "apperror/apperror.h"
#include "jsonresponse/jsonerrorrequestobjectFormatschema.h"
#include "jsonresponse/jsonresponse.h"
#include "users/db/createuserdb.h"
#include "users/schemas/createuserschema.h"
#include "utility/utility.h"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/parser_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/impl/serialize.hpp>
#include <boost/json/parse.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <iostream>
#include <openssl/x509v3.h>

namespace CreateUserRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline createUserRoute(boost::beast::tcp_stream
                                                       &socket_stream,
                                                   boost::beast::http::
                                                       request_parser<
                                                           boost::beast::http::
                                                               empty_body>
                                                           &request,
                                                   boost::beast::flat_buffer
                                                       &flatBuffer,
                                                   boost::mysql::connection_pool
                                                       &connPool) {
  boost::beast::http::request_parser<boost::beast::http::string_body>
      requestString{std::move(request)};

  boost::beast::error_code ec;

  auto rz = co_await boost::beast::http::async_read(
      socket_stream, flatBuffer, requestString,
      boost::asio::redirect_error(ec));

  // std::cout << " after reading" << std::endl;
  if (ec) {
    std::cerr << "error from reading inside /api/user/" << std::endl;
    std::cerr << ec.what() << std::endl;

    auto erc = makeErrorCode(AppError::Reading_Request_Failed);
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(), erc);
  }

  auto userExpectedObj =
      Utility::convertBodyToObject<CreateUserSchema>(requestString.get());

  // if the body of the request could not be converted into object
  //  1. send a error format on how the request should be.
  if (!userExpectedObj.has_value()) {
    JsonErrorRequestObjectFormatSchema<CreateUserSchema> sendUserSchema;
    co_return JsonResponse::sendJsonBadObjectFormatResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        sendUserSchema);
  }

  auto sendUserSchema = userExpectedObj.value();

  auto createUserBdAction =
      co_await CreateUserDB::createUserDB(connPool, sendUserSchema);

  if (createUserBdAction.has_value())
    co_return JsonResponse::sendJsonResponse(requestString.get().version(),
                                             requestString.get().keep_alive(),
                                             createUserBdAction.value());

  co_return JsonResponse::sendJsonBadResponse(requestString.get().version(),
                                              requestString.get().keep_alive(),
                                              createUserBdAction.error());
}
}; // namespace CreateUserRoute