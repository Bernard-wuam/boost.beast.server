
#pragma once
#include "apperror/apperror.h"
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
#include <boost/json/value_from.hpp>
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
#include <optional>
#include <sodium/crypto_pwhash.h>
#include <system_error>

class GetPostCommentDB {
public:
  static boost::asio::awaitable<
      std::expected<boost::json::value, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  getPostCommentDB(boost::mysql::connection_pool &conn,
                   const std::string &index,
                   const std::optional<JwtUserInfoSchema> &user) {
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

    std::string public_id = "";

    if (user.has_value())
      public_id = user.value().public_id;

    co_await getCon->async_execute(boost::mysql::with_params(
                                       R"(
                                                  SELECT JSON_ARRAYAGG(JSON_OBJECT('isyourcomment',u.public_id = {0},'content',c.content, 'created_at',c.created_at, 'username',u.username,
                                                  'reply_to',IF( c.reply_to is NULL, NULL, JSON_OBJECT('content',rc.content,'created_at', rc.created_at,'username', uu.username))))
                                                  FROM comments c INNER JOIN users u ON c.user_id = u.user_id
                                                  LEFT JOIN comments rc ON rc.comment_id = c.reply_to
                                                  LEFT JOIN users uu ON uu.user_id = rc.user_id WHERE c.post_id =
                                                  (SELECT c.post_id FROM posts WHERE public_id = {1}) ORDER BY c.created_at;
                              )",
                                       public_id, index),
                                   result, diag,
                                   boost::asio::redirect_error(ec));
    getCon.return_without_reset();

    if (ec) {
      std::cout << "from get post" << std::endl;
      std::cerr << ec.what() << std::endl;
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.rows().at(0).empty() || result.rows().at(0).at(0).is_null())
      co_return std::unexpected(makeErrorCode(AppError::Post_Not_Found));

    auto val = result.rows().at(0).at(0).as_string();

    auto obj = boost::json::parse(val, ec);

    if (ec) {
      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    co_return obj;
  }
};