
#pragma once

#include "apperror/apperror.h"
#include "jsonresponse/jsonresponse.h"
#include "security/security.h"
#include "users/db/getuserdb.h"
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

namespace GetUserRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline getUserRoute(boost::beast::tcp_stream
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
  // get the body of the request
  // if it fails return a http response with the error
  auto userExpectedObj = Security::verifyJwt(requestString.get(), SECRETEKEY);

  if (!userExpectedObj.has_value()) {
    co_return JsonResponse::sendJsonBadResponse(requestString.get().version(),
                                                false, userExpectedObj.error());
  }

  boost::beast::error_code ec;

  auto rz = co_await boost::beast::http::async_read(
      socket_stream, flatBuffer, requestString,
      boost::asio::redirect_error(ec));

  // std::cout << " after reading" << std::endl;
  if (ec) {
    std::cerr << "error from reading inside /api/users" << std::endl;
    std::cerr << ec.what() << std::endl;

    auto erc = makeErrorCode(AppError::Reading_Request_Failed);
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(), erc);
  }

  auto user = userExpectedObj.value();
  auto getUserDbAction = co_await GetUserDB::getUserDB(connPool, user);

  if (!getUserDbAction.has_value()) {
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        getUserDbAction.error());
  }

  co_return JsonResponse::sendJsonResponse(requestString.get().version(),
                                           requestString.get().keep_alive(),
                                           getUserDbAction.value());
}
} // namespace GetUserRoute