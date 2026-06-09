
#pragma once
#include "apperror/apperror.h"
#include "posts/schemas/editpostschema.h"
#include "posts/schemas/jwtuserinfoschema.h"
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

class EditPostDB {
public:
  static boost::asio::awaitable<
      std::expected<boost::json::object, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  editPostDB(boost::mysql::connection_pool &conn, const std::string &index,
             const JwtUserInfoSchema &user,
             const EditPostSchema &editPostSchema) {
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
    boost::mysql::static_results<std::tuple<>, std::tuple<std::string>> result;

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "START TRANSACTION;"
            "SELECT users.public_id FROM users INNER JOIN posts ON "
            "users.user_id = posts.poster_user_id WHERE posts.public_id = {}; ",
            index),
        result, diag, boost::asio::redirect_error(ec));

    if (ec) {
      std::cout << "from get post" << std::endl;
      std::cerr << ec.what() << std::endl;
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.rows<1>().empty())
      co_return std::unexpected(makeErrorCode(AppError::Post_Not_Found));

    if (user.public_id == std::get<0>(result.rows<1>()[0]) ||
        user.role == "admin") {
      boost::mysql::results result;

      co_await getCon->async_execute(
          boost::mysql::with_params(R"(
                                                    UPDATE posts 
                                                    SET posts.title = {0},
                                                    posts.content = {1}
                                                    WHERE posts.public_id = {2};

                                                    UPDATE posts_categories pc 
                                                    INNER JOIN posts p ON p.post_id = pc.post_id 
                                                    SET pc.categories_id = (SELECT categories.categories_id 
                                                    FROM categories WHERE categories.name = {3})
                                                    WHERE p.public_id = {2};
                                                    
                                                    COMMIT;
                                                  )",
                                    editPostSchema.title,
                                    editPostSchema.content, index,
                                    editPostSchema.category),
          result, diag, boost::asio::redirect_error(ec));
      getCon.return_without_reset();

      if (ec) {
        std::cout << "from inside user and admin" << std::endl;
        std::cerr << ec.what() << std::endl;
        if (!diag.server_message().empty())
          std::cerr << diag.server_message() << std::endl;

        co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
      }
      co_return boost::json::object{{"result", "updated"}};
    }

    co_return std::unexpected(makeErrorCode(AppError::Illegal_post_Operation));
  }
};