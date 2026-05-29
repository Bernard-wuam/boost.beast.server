
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
#include <boost/json/value.hpp>
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
#include <sodium/crypto_pwhash.h>
#include <system_error>
#include <tuple>

class DeleteDisLikeDB {
public:
  static boost::asio::awaitable<
      std::expected<boost::json::value, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  deleteDisLikeDB(boost::mysql::connection_pool &conn, const std::string &index,
                  const JwtUserInfoSchema &user) {
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
    boost::mysql::static_results<std::tuple<>> result;

    co_await getCon->async_execute(boost::mysql::with_params(
                                       R"(                                  
                                                  DELETE l FROM comment_affinities l INNER JOIN users u
                                                  ON l.user_id = u.user_id
                                                  INNER JOIN comments c ON l.comment_id = c.comment_id
                                                  WHERE u.public_id = {} 
                                                  AND c.comment_public_id = {};
                                               )",
                                       user.public_id, index),
                                   result, diag,
                                   boost::asio::redirect_error(ec));
    if (ec) {
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }
      std::cerr << ec.what() << std::endl;
      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    boost::json::value obj(boost::json::object{{"result", "deleted"}});

    co_return obj;
  }
};