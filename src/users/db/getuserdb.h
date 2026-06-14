
#pragma once
#include "apperror/apperror.h"
#include "posts/schemas/jwtuserinfoschema.h"
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

class GetUserDB {
public:
  static boost::asio::awaitable<
      std::expected<boost::json::object, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  getUserDB(boost::mysql::connection_pool &conn,
            const JwtUserInfoSchema &user) {

    boost::beast::error_code ec;

    auto getCon =
        co_await conn.async_get_connection(boost::asio::redirect_error(ec));
    if (ec) {
      std::cerr << ec << std::endl;
      co_return std::unexpected(
          makeErrorCode(AppError::No_Database_Connection));
    }

    boost::mysql::diagnostics diag;
    boost::mysql::results result;

    // get the password from the db;
    // if the result is empty that means the user is not found.
    co_await getCon->async_execute(boost::mysql::with_params(
                                       R"(         
                                                    SELECT JSON_OBJECT( 'profile_picture',users.profile_picture,
                                                    'username',users.username,'email', users.email) 
                                                    FROM users WHERE users.public_id = {};
                                                )",
                                       user.public_id),
                                   result, diag,
                                   boost::asio::redirect_error(ec));

    if (ec) {
      std::cerr << ec.what() << " from 1" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.at(0).rows().empty())
      co_return std::unexpected(makeErrorCode(AppError::User_Not_Found));

    auto resObject =
        boost::json::parse(result.at(0).rows().at(0).at(0).as_string(), ec);

    if (ec) {
      std::cerr << ec.what() << "could not convert to string" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    co_return resObject.as_object();
  }
};