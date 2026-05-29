#pragma once

#include "apperror/apperror.h"
#include "jsonresponse/jsonresponse.h"
#include "posts/db/getpostbycategorydb.h"
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
#include <boost/url/urls.hpp>
#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <openssl/x509v3.h>

namespace GetPostByCategoryRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline getPostByCategoryRoute(boost::beast::
                                                              tcp_stream &
                                                                  socket_stream,
                                                          boost::beast::http::
                                                              request_parser<
                                                                  boost::beast::
                                                                      http::
                                                                          empty_body>
                                                                  &request,
                                                          boost::beast::
                                                              flat_buffer
                                                                  &flatBuffer,
                                                          boost::mysql::
                                                              connection_pool
                                                                  &connPool) {
  // check the header if the user is authenticated before they can post.

  boost::beast::http::request_parser<boost::beast::http::string_body>
      requestString{std::move(request)};
  requestString.body_limit(boost::none);
  std::cout << requestString.get()[boost::beast::http::field::content_type]
            << std::endl;
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

  boost::url_view urlView(requestString.get().target());
  auto urlParams = urlView.params();
  std::cout << urlParams.contains("category") << " : does it contain categories"
            << std::endl;
  auto val = urlParams.find("category");
  auto category = (*val).value;
  auto getPostByCategoryDbAction =
      (co_await GetPostByCategoryDB::getPostByCategoryDB(connPool, category));

  if (!getPostByCategoryDbAction.has_value())
    co_return JsonResponse::sendJsonBadResponse(
        requestString.get().version(), requestString.get().keep_alive(),
        getPostByCategoryDbAction.error());

  co_return JsonResponse::sendJsonResponse(requestString.get().version(),
                                           requestString.get().keep_alive(),
                                           getPostByCategoryDbAction.value());
}
} // namespace GetPostByCategoryRoute