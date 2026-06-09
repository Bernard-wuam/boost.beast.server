#include "category/routes/getcategoryroute.h"
#include "comments/routes/createpostcommentroute.h"
#include "comments/routes/deletecommentroute.h"
#include "comments/routes/getcommentroute.h"
#include "comments/routes/getpostcommentroute.h"
#include "comments/routes/updatecommentroute.h"
#include "likes/routes/createdislikeroute.h"
#include "likes/routes/createlikeroute.h"
#include "likes/routes/deletelikeroute.h"
#include "posts/routes/createpostroute.h"
#include "posts/routes/deletepostroute.h"
#include "posts/routes/editpostroute.h"
#include "posts/routes/getallpostroute.h"
#include "posts/routes/getimageroute.h"
#include "posts/routes/getpostbycategoryroute.h"
#include "posts/routes/getpostbyuserroute.h"
#include "posts/routes/getpostroute.h"
#include "posts/routes/uploadroute.h"
#include "regexrouter/regexrouter.h"
#include "users/routes/createuserroute.h"
#include "users/routes/loginuserroute.h"
#include "users/routes/refreshuserroute.h"
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/buffers_generator.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/parser_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <boost/none.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/url.hpp>
#include <boost/url/parse.hpp>
#include <iostream>

#pragma once

class Server {
public:
  boost::asio::awaitable<
      boost::beast::http::message_generator,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  handleMessage(
      boost::beast::http::request_parser<boost::beast::http::empty_body>
          &request,
      boost::beast::flat_buffer &_flat_buf,
      boost::beast::tcp_stream &socket_stream,
      boost::mysql::connection_pool &connPool) {

    // do some security checks here
    // .1. check for any ... in the path
    // .2. check, if the path does not start with a /
    // .3. check if the path is empty

    if (request.get().target() == "/api/user" &&
        request.get().method() == boost::beast::http::verb::post) {

      co_return (co_await CreateUserRoute::createUserRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    if (request.get().target() == "/api/auth/login" &&
        request.get().method() == boost::beast::http::verb::post) {

      co_return (co_await LoginUserRoute::loginUserRoute(socket_stream, request,
                                                         _flat_buf, connPool));
    }

    if (request.get().target() == "/api/auth/refresh" &&
        request.get().method() == boost::beast::http::verb::post) {

      co_return (co_await RefreshUserRoute::refreshUserRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    // handle the post route at this section of the code.
    //............POST............................POST..................................POST....................
    if (request.get().target() == "/api/posts" &&
        request.get().method() == boost::beast::http::verb::post) {
      std::cout << "post route reached" << std::endl;
      co_return (co_await CreatePostRoute::createPostRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    if (request.get().target() == "/api/posts/upload" &&
        request.get().method() == boost::beast::http::verb::post) {
      std::cout << "/api/post/upload" << std::endl;
      co_return (co_await UploadRoute::uploadRoute(socket_stream, request,
                                                   _flat_buf, connPool));
    }
    // get a single post route and delete a single post route.

    if (request.get().target() == "/api/user/image/upload" &&
        request.get().method() == boost::beast::http::verb::post) {
      std::cout << "/api/user/image/upload" << std::endl;
      co_return (co_await UploadRoute::uploadRoute(socket_stream, request,
                                                   _flat_buf, connPool));
    }
    // get a single post route and delete a single post route.

    RegexPath regexPath(
        R"(/api/posts/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})$)");

    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      co_return (co_await GetPostRoute::getPostRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::delete_) {
      co_return (co_await DeletePostRoute::deletePostRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::put) {
      co_return (co_await EditPostRoute::editPostRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    // get a single post route and delete a single post route.

    regexPath.setRegexUrl(
        R"(/api/posts\?user=([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})$)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      co_return (co_await GetPostByUserRoute::getPostByUserRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    // /api/posts/\?categories=([a-zA-Z])+
    // get a all post by category.

    regexPath.setRegexUrl(R"(/api/posts\?category=([a-zA-Z])+)");

    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      std::cout << "get post by categories" << std::endl;
      co_return (co_await GetPostByCategoryRoute::getPostByCategoryRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    if (request.get().target() == "/api/posts" &&
        request.get().method() == boost::beast::http::verb::get) {
      co_return (co_await GetAllPostRoute::getAllPostRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    //...................................categories......................................................

    if (request.get().target() == "/api/category" &&
        request.get().method() == boost::beast::http::verb::get) {
      std::cout << "/api/post/upload" << std::endl;
      co_return (co_await GetCategoryRoute::getCategoryRoute(
          socket_stream, request, _flat_buf, connPool));
    }

    //.....................................comments..................................................................

    regexPath.setRegexUrl(
        R"(/api/posts/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/comments)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      co_return (co_await GetPostCommentRoute::getPostCommentRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/posts/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/comments)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::post) {
      co_return (co_await CreatePostCommentRoute::createPostCommentRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}))");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      co_return (co_await GetCommentRoute::getCommentRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}))");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::delete_) {
      co_return (co_await DeleteCommentRoute::deleteCommentRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}))");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::put) {
      co_return (co_await UpdateCommentRoute::updateCommentRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    //.....................................likes..................................................................

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/likes)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::post) {
      co_return (co_await CreateLikeRoute::createLikeRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/likes)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::delete_) {
      co_return (co_await DeleteLikeRoute::deleteLikeRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/dislikes)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::post) {
      co_return (co_await CreateDisLikeRoute::createDisLikeRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    regexPath.setRegexUrl(
        R"(/api/comments/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})/dislikes)");
    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::delete_) {
      co_return (co_await CreateDisLikeRoute::createDisLikeRoute(
          socket_stream, request, _flat_buf, connPool, regexPath));
    }

    //.....................................images........................................................

    regexPath.setRegexUrl(
        R"(/api/posts/image/([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})\.(jpg|png|jpeg|svg|gif|avif|webp)$)");

    if (regexPath.isMatched(request.get().target()) &&
        request.get().method() == boost::beast::http::verb::get) {
      std::cout << "images route is hit" << std::endl;
      co_return (co_await GetImageRoute::getImageRoute(socket_stream, request));
    }

    std::cout << "reached home route" << std::endl;
    boost::beast::http::response<boost::beast::http::string_body> res(
        boost::beast::http::status::ok, request.get().version());
    res.set(boost::beast::http::field::content_type, "application/json");
    res.keep_alive(false);
    res.body() = R"({"result":"welcome"})";
    res.prepare_payload();
    co_return res;
  }

  boost::asio::awaitable<
      void, boost::asio::strand<boost::asio::io_context::executor_type>>
  handleSession(boost::asio::ip::tcp::socket &&socket,
                boost::mysql::connection_pool &connPool) {
    boost::beast::tcp_stream socket_stream(std::move(socket));
    boost::system::error_code ec;

    // socket_stream.expires_after(std::chrono::seconds(5));

    for (;;) {
      std::cout << "start reading cyble" << std::endl;
      boost::beast::flat_buffer fl_buffer;
      boost::beast::http::request_parser<boost::beast::http::empty_body>
          request_parser;
      request_parser.body_limit(boost::none);
      // request_parser.skip(true);

      auto req_size = co_await boost::beast::http::async_read_header(
          socket_stream, fl_buffer, request_parser,
          boost::asio::redirect_error(ec));

      if (ec) {
        if (ec == boost::beast::http::error::end_of_stream ||
            ec == boost::asio::error::operation_aborted)
          co_return;
        std::cerr << "error from reading in server" << std::endl;
        std::cerr << ec.what() << std::endl;
        // std::cerr << ec.location() << std::endl;
        // throw boost::system::system_error(ec);
        co_return;
      }
      std::cout << "no error after reading header" << std::endl;
      ec.clear();
      auto k = co_await handleMessage(request_parser, fl_buffer, socket_stream,
                                      connPool);

      auto keepAlive = k.keep_alive();
      auto writeSize = co_await boost::beast::async_write(
          socket_stream, std::move(k), boost::asio::redirect_error(ec));
      if (ec) {
        std::cout << "writing error " << std::endl;
        std::cerr << ec.what() << std::endl;
        co_return;
      }
      std::cout << "after writing" << std::endl;
      std::cout << keepAlive << " :keep alive" << std::endl;

      if (!keepAlive) {
        std::cout << "not keep alive" << std::endl;
        co_return;
      }
    }
    co_return;
  };

  boost::asio::awaitable<
      void, boost::asio::strand<boost::asio::io_context::executor_type>>
  startServer(boost::asio::ip::tcp::endpoint ep,
              boost::mysql::connection_pool &connPool) {
    std::cout << "hello coroutine ..." << std::endl;
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor(executor, ep);

    boost::system::error_code ec;

    for (;;) {
      auto newStrand = boost::asio::make_strand(executor.get_inner_executor());
      auto socket = co_await acceptor.async_accept(
          newStrand, boost::asio::redirect_error(ec));

      // throw boost::system::system_error(ec);

      if (ec) {
        std::cerr << ec.what() << std::endl;
        // log to a logger
        co_return;
      }

      std::cout << "server started sucessfully.." << std::endl;

      boost::asio::co_spawn(std::move(newStrand),
                            handleSession(std::move(socket), connPool),
                            boost::asio::detached);
      ec.clear();
    }

    co_return;
  }
}; // namespace server