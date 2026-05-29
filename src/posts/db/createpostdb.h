
#pragma once
#include "apperror/apperror.h"
#include "posts/schemas/createpostschema.h"
#include "posts/schemas/jwtuserinfoschema.h"
#include "posts/schemas/sendpostschema.h"
#include <bcrypt/BCrypt.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/impl/error.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <boost/json.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>
#include <boost/mysql.hpp>
#include <boost/mysql/connection_pool.hpp>
#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/results.hpp>
#include <boost/mysql/static_results.hpp>
#include <boost/mysql/with_params.hpp>
#include <boost/system/detail/errc.hpp>
#include <boost/system/system_error.hpp>
#include <boost/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <expected>
#include <iostream>
#include <openssl/x509v3.h>
#include <sodium/crypto_pwhash.h>
#include <system_error>

class CreatePostDB {
public:
  static boost::asio::awaitable<
      std::expected<SendPostSchema, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  createPostDB(boost::mysql::connection_pool &conn,
               const CreatePostSchema &post, const JwtUserInfoSchema &user) {
    std::cout << "create post db reached" << std::endl;
    boost::beast::error_code ec;

    auto getCon =
        co_await conn.async_get_connection(boost::asio::redirect_error(ec));
    if (ec) {
      std::cerr << ec << std::endl;
      co_return std::unexpected(
          makeErrorCode(AppError::No_Database_Connection));
    }

    // do dabase action.
    // write the user to the database
    // if something goes wrong return an error code.
    boost::mysql::diagnostics diag;

    boost::mysql::static_results<std::tuple<>, std::tuple<std::uint64_t>>
        result;

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "START TRANSACTION;"
            "SELECT categories_id FROM categories WHERE name = {};",
            post.category),
        result, diag, boost::asio::redirect_error(ec));

    if (ec) {
      std::cerr << ec.what() << "from   1" << std::endl;
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.rows<1>().empty()) {
      co_return std::unexpected(makeErrorCode(AppError::Category_Not_Found));
    }

    auto category_id = std::get<0>(result.rows<1>()[0]);

    boost::mysql::static_results<std::tuple<>, SendPostSchema, std::tuple<>,
                                 std::tuple<>>
        result2;

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "INSERT INTO posts(title,content,poster_user_id,image_url) "
            "VALUES({0},{1},(SELECT user_id FROM users WHERE public_id = "
            "{2}),{4});"
            "SELECT title,content,created_at,last_updated FROM posts WHERE "
            "post_id = LAST_INSERT_ID();"
            "INSERT INTO posts_categories(post_id,categories_id) "
            "VALUES(LAST_INSERT_ID(),{3});"
            "COMMIT;",
            post.title, post.content, user.public_id, category_id,
            post.image_url),
        result2, diag, boost::asio::redirect_error(ec));
    getCon.return_without_reset();
    if (ec) {
      std::cerr << ec.what() << std::endl << "from 2";
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }
    co_return result2.rows<1>()[0];
  }
};