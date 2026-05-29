
#pragma once
#include "apperror/apperror.h"
#include "users/schemas/createuserschema.h"
#include "users/schemas/senduserschema.h"
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

class CreateUserDB {
public:
  static boost::asio::awaitable<
      std::expected<SendUserSchema, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  createUserDB(boost::mysql::connection_pool &conn, CreateUserSchema &user) {
    boost::beast::error_code ec;

    auto getCon =
        co_await conn.async_get_connection(boost::asio::redirect_error(ec));
    if (ec) {
      std::cerr << ec << std::endl;
      co_return std::unexpected(
          makeErrorCode(AppError::No_Database_Connection));
    }

    // try hashing the password
    // if somthing goes wrong, return the error code from the
    // Security::hashPassword function.
    //  else return the password in string.
    // auto hashPassWordExpected = Security::hashPassword(user.password);

    // if (!hashPassWordExpected.has_value()) {
    //   co_return std::unexpected(hashPassWordExpected.error());
    // }

    // std::string password{hashPassWordExpected.value().begin(),
    //                      hashPassWordExpected.value().end()};

    std::string pasw = user.password;

    // do dabase action.
    // write the user to the database
    // if something goes wrong return an error code.
    boost::mysql::diagnostics diag;
    boost::mysql::static_results<std::tuple<>, std::tuple<>, SendUserSchema,
                                 std::tuple<>>
        result;

    char passwordHash[crypto_pwhash_STRBYTES];
    auto isCorrect = crypto_pwhash_str(passwordHash, pasw.c_str(), pasw.size(),
                                       crypto_pwhash_OPSLIMIT_INTERACTIVE,
                                       crypto_pwhash_MEMLIMIT_INTERACTIVE);

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "START TRANSACTION;"
            "INSERT INTO users "
            "(username,email,hashed_password) "
            "VALUES({0},{1},{2});"
            "SELECT username,email,public_id,created_at FROM users WHERE user_id "
            "= "
            "LAST_INSERT_ID();"
            "COMMIT;",
             user.username, user.email, passwordHash),
        result, diag, boost::asio::redirect_error(ec));

    getCon.return_without_reset();

    if (ec) {
      std::cerr << ec.what() << std::endl;
      // std::cerr << diag.server_message() << std::endl;

      // construct error string for if.
      // 1. the username already exist in the db.
      // 2.the email already exist in the db.
      //  return the appropriate error code
      if (!diag.server_message().empty()) {
        auto usernameError = std::format(
            "Duplicate entry '{}' for key 'users.idx_username'", user.username);

        auto emailError = std::format(
            "Duplicate entry '{}' for key 'users.idx_email'", user.email);

        if (diag.server_message() == usernameError)
          co_return std::unexpected(
              makeErrorCode(AppError::Username_Already_In_use));

        if (diag.server_message() == emailError)
          co_return std::unexpected(
              makeErrorCode(AppError::Email_Already_In_Use));
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    co_return (result.rows<2>()[0]);
  }
};