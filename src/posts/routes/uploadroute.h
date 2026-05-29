#pragma once

#include "apperror/apperror.h"
#include "jsonresponse/jsonresponse.h"
#include <algorithm>
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
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <boost/none.hpp>
#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <iostream>
#include <openssl/x509v3.h>

namespace UploadRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::string_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline uploadRoute(boost::beast::tcp_stream
                                                   &socket_stream,
                                               boost::beast::http::
                                                   request_parser<
                                                       boost::beast::http::
                                                           empty_body> &request,
                                               boost::beast::flat_buffer
                                                   &flatBuffer,
                                               boost::mysql::connection_pool
                                                   &connPool) {
  // check the header if the user is authenticated before they can post.

  boost::beast::http::request_parser<boost::beast::http::file_body>
      requestString{std::move(request)};
  requestString.body_limit(boost::none);

  boost::beast::error_code ec;
  // extract the image from the file body i.e dynamic body and save to
  // assets/images/filename.jpg
  std::filesystem::path filePath(CURRENTPATH);
  boost::uuids::random_generator randGen;

  std::string filename = boost::uuids::to_string(randGen());

  std::string mimetype = requestString.get()
                             .find(boost::beast::http::field::content_type)
                             ->value();
  auto index = mimetype.rfind("/");
  std::string mimeExtension = "." + mimetype.substr(index + 1, mimetype.size());
  
  filename.append(mimeExtension);
  filePath.append("assets/posts/image").append(filename);

  boost::json::object imageDetails{
      {"postImage", std::string{"posts/image/"} + filename}};

  requestString.get().body().open(filePath.c_str(),
                                  boost::beast::file_mode::write, ec);
  if (ec) {
    std::cout << ec.what() << std::endl;

    auto erc = makeErrorCode(AppError::Reading_Request_Failed);
    co_return JsonResponse::sendJsonBadResponse(requestString.get().version(),
                                                false, erc);
  }

  auto rz = co_await boost::beast::http::async_read(
      socket_stream, flatBuffer, requestString,
      boost::asio::redirect_error(ec));

  // std::cout << " after reading" << std::endl;
  if (ec) {
    std::cerr << "error from reading inside /api/post/upload/" << std::endl;
    std::cerr << ec.what() << std::endl;

    auto erc = makeErrorCode(AppError::Reading_Request_Failed);
    co_return JsonResponse::sendJsonBadResponse(requestString.get().version(),
                                                false, erc);
  }

  co_return JsonResponse::sendJsonResponse(requestString.get().version(),
                                           requestString.get().keep_alive(),
                                           imageDetails);
};
} // namespace UploadRoute