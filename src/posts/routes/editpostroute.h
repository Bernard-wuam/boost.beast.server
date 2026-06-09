#pragma once

#include "apperror/apperror.h"
#include "jsonresponse/jsonerrorrequestobjectFormatschema.h"
#include "jsonresponse/jsonresponse.h"
#include "posts/db/editpostdb.h"
#include "posts/schemas/editpostschema.h"
#include "regexrouter/regexrouter.h"
#include "security/security.h"
#include "utility/utility.h"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/file_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/parser_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/impl/serialize.hpp>
#include <boost/json/parse.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <boost/none.hpp>
#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <openssl/x509v3.h>
#include <optional>
#include <regex>

namespace EditPostRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline editPostRoute(boost::beast::tcp_stream
                                                       &socket_stream,
                                                   boost::beast::http::
                                                       request_parser<
                                                           boost::beast::http::
                                                               empty_body>
                                                           &request,
                                                   boost::beast::flat_buffer
                                                       &flatBuffer,
                                                   boost::mysql::connection_pool
                                                       &connPool,
                                                   const RegexPath &regexPath) {
  // check the header if the user is authenticated before they can post.


  auto userExpected = Security::verifyJwt(request.get(), SECRETEKEY);

  if (!userExpected.has_value())
       co_return JsonResponse::sendJsonBadResponse(
        request.get().version(), false,
        userExpected.error());

  boost::beast::http::request_parser<boost::beast::http::string_body>
      requestString{std::move(request)};
  requestString.body_limit(boost::none);
  boost::beast::error_code ec;

  auto rz = co_await boost::beast::http::async_read(
      socket_stream, flatBuffer, requestString,
      boost::asio::redirect_error(ec));

  // std::cout << " after reading" << std::endl;
  if (ec) {
    std::cerr << "error from reading inside /api/post/" << std::endl;
    std::cerr << ec.what() << std::endl;

    auto erc = makeErrorCode(AppError::Reading_Request_Failed);
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(), erc);
  }

  auto indexExpected = regexPath.extraPathId(std::regex(
      "([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})$"));

  if (!indexExpected.has_value()) {

    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        indexExpected.error());
  }

    auto objectBodyExpected =
      Utility::convertBodyToObject<EditPostSchema>(requestString.get());

  if (!objectBodyExpected.has_value()) {
    JsonErrorRequestObjectFormatSchema<EditPostSchema> errformat;
    co_return JsonResponse::sendJsonBadObjectFormatResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        errformat);
  }

  auto editPostDbAction = (co_await EditPostDB::editPostDB(
      connPool, indexExpected.value(), userExpected.value(),objectBodyExpected.value()));

  if (!editPostDbAction.has_value())
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        editPostDbAction.error());

  co_return JsonResponse::sendJsonResponse(requestString.get().version(),
                                           requestString.get().keep_alive(),
                                           editPostDbAction.value());
}
} // namespace EditPostRoute