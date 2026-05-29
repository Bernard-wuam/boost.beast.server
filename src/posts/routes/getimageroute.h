#pragma once

#include "apperror/apperror.h"
#include "jsonresponse/jsonresponse.h"
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

namespace GetImageRoute {

boost::asio::awaitable<
    boost::beast::http::response<boost::beast::http::file_body>,
    boost::asio::strand<
        boost::asio::io_context::
            executor_type>> inline getImageRoute(boost::beast::tcp_stream
                                                     &socket_stream,
                                                 boost::beast::http::
                                                     request_parser<
                                                         boost::beast::http::
                                                             empty_body>
                                                         &request/*,
                                                 boost::beast::flat_buffer
                                                     &flatBuffer,
                                                 const RegexPath &regexPath*/) {
  // check the header if the user is authenticated before they can post.

  boost::beast::error_code ec;
  // extract the image from the file body i.e dynamic body and save to
  // assets/images/filename.jpg
  std::filesystem::path filePath(CURRENTPATH);
  std::string target = (request.get().target());

  std::cout << target << " : the target" << std::endl;

  auto rIndex = target.find("posts");

  //   if (rIndex == std::string::npos)
  //     co_return JsonResponse::sendJsonBadResponse(request.get().version(),
  //                                                 request.get().keep_alive(),
  //                                                 AppError::Server_Failed);

  auto newTarget = target.substr(rIndex, target.length());

  auto imageExtention = Utility::getImageExtention(target);

  std::cout << imageExtention << " : the extension" << std::endl;

  //   filePath.append("assets");
  filePath.append("assets").append(newTarget);

  std::cout << filePath.c_str() << " :the file path " << std::endl;

  boost::beast::http::response<boost::beast::http::file_body> response;
  response.version(request.get().version());
  response.keep_alive(request.get().keep_alive());
  response.set(boost::beast::http::field::content_type, imageExtention);

  response.body().open(filePath.c_str(), boost::beast::file_mode::scan, ec);
  response.prepare_payload();

  if (ec) {
    std::cerr << ec.what() << std::endl;
    std::filesystem::path path(CURRENTPATH);
    path.append("assets").append("posts").append("image").append(
        "no_image.jpg");

    std::cout << path.c_str() << std::endl;

    auto extension = Utility::getImageExtention(path.c_str());

    boost::beast::http::response<boost::beast::http::file_body> response;
    response.version(request.get().version());
    response.keep_alive(request.get().keep_alive());
    response.body().open(path.c_str(), boost::beast::file_mode::scan, ec);
    response.set(boost::beast::http::field::content_type, extension);
    response.prepare_payload();
    co_return response;
  }

  co_return response;
}
} // namespace GetImageRoute