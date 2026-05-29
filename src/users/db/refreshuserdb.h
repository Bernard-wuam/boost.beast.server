
#pragma once
#include "apperror/apperror.h"
#include "users/schemas/sendrefreshandaccesstokenschema.h"
#include "security/security.h"
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

class RefreshUserDB {
public:
  static boost::asio::awaitable<
      std::expected<SendRefreshAndAcessTokenSchema, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  refreshUserDB(boost::mysql::connection_pool &conn,
                const std::string &refreshToken) {
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

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "START TRANSACTION;"

            "SELECT ( NOW() > users_refresh_token.expires_at ), "
            "users.public_id, "
            "users.role, users.user_id from users "
            "INNER JOIN "
            "users_refresh_token  ON users.user_id = "
            "users_refresh_token.refresh_token_user_id "
            "WHERE users_refresh_token.refresh_token = {};",
            refreshToken),
        result, diag, boost::asio::redirect_error(ec));

    if (ec) {
      std::cerr << ec.what() << "from 1" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.at(1).rows().empty()) {
      co_return std::unexpected(makeErrorCode(AppError::Invalid_RefreshToken));
    }

    auto isExpired = static_cast<bool>(result.at(1).rows()[0][0].as_int64());

    if (isExpired) {
      co_return std::unexpected(makeErrorCode(AppError::RefreshToken_Expired));
    }

    auto public_id = result.at(1).rows()[0][1].as_string();
    auto role = result.at(1).rows()[0][2].as_string();
    auto user_id = result.at(1).rows()[0][3].as_uint64();

    boost::mysql::static_results<std::tuple<>, std::tuple<>,
                                 std::tuple<std::string>, std::tuple<>>
        result2;

    co_await getCon->async_execute(
        boost::mysql::with_params(
            "DELETE FROM users_refresh_token WHERE refresh_token_user_id = "
            "{0};  "

            "INSERT INTO users_refresh_token(refresh_token_user_id,expires_at) "
            "VALUES({0},current_timestamp() + INTERVAL 20 SECOND);"

            "SELECT refresh_token FROM users_refresh_token "
            "WHERE "
            "refresh_token_id = LAST_INSERT_ID();"

            "COMMIT;",
            user_id),
        result2, diag, boost::asio::redirect_error(ec));
    getCon.return_without_reset();

    if (ec) {
      std::cerr << ec.what() << "from 2" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    auto newRefreshToken = std::get<0>(result2.rows<2>()[0]);

    auto accessToken = Security::createJwt(
        boost::json::object{{"public_id", public_id}, {"role", role}},
        std::chrono::system_clock::now() + std::chrono::seconds(30), "");

    co_return SendRefreshAndAcessTokenSchema(accessToken,newRefreshToken) ;
  }
};