
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
#include <boost/json/value.hpp>
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
      std::expected<boost::json::value, std::error_code>,
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

    boost::mysql::results result;

    co_await getCon->async_execute(
        boost::mysql::with_params(
            R"(
                      START TRANSACTION;

                      INSERT INTO posts(public_id,title,content,poster_user_id,image_url) 
                      VALUES({0},{1},{2},(SELECT user_id FROM users WHERE public_id = 
                      {3}),{4});

                      SELECT JSON_OBJECT('title',posts.title,'content',posts.content,'created_at',posts.created_at,
                      'last_updated',posts.last_updated,'image_url',posts.image_url) FROM posts WHERE 
                      post_id = LAST_INSERT_ID();

                      INSERT INTO posts_categories(post_id,categories_id) 
                      VALUES(LAST_INSERT_ID(),(SELECT categories_id FROM categories WHERE name = {5}));
                      
                      COMMIT;
                    )",
            post.public_id, post.title, post.content, user.public_id,
            post.image_url, post.category),
        result, diag, boost::asio::redirect_error(ec));
    getCon.return_without_reset();

    if (ec) {
      std::cerr << ec.what() << std::endl << "from 2";
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.at(2).rows().empty()) {
      co_return std::unexpected(makeErrorCode(AppError::Post_Not_Found));
    };

    auto val =
        boost::json::parse(result.at(2).rows().at(0).at(0).as_string(), ec);

    if (ec) {
      std::cerr << ec.what() << std::endl;
      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }
    co_return val.as_object();
  }
};