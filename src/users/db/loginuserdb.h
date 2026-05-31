
#pragma once
#include "apperror/apperror.h"
#include "security/security.h"
#include "users/schemas/loginsucessfullschema.h"
#include "users/schemas/loginuserschema.h"
#include "users/schemas/sendloginuserschema.h"
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
#include <chrono>
#include <expected>
#include <iostream>
#include <openssl/x509v3.h>
#include <sodium/crypto_pwhash.h>
#include <system_error>

class LoginUserDB {
public:
  static boost::asio::awaitable<
      std::expected<LoginSuccessFullSchema, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  loginUserDB(boost::mysql::connection_pool &conn, LoginUserSchema &user) {

    boost::beast::error_code ec;

    auto getCon =
        co_await conn.async_get_connection(boost::asio::redirect_error(ec));
    if (ec) {
      std::cerr << ec << std::endl;
      co_return std::unexpected(
          makeErrorCode(AppError::No_Database_Connection));
    }

    boost::mysql::diagnostics diag;
    boost::mysql::static_results<std::tuple<>,

                                 std::tuple<std::string>, SendLoginUserSchema>
        result;

    // get the password from the db;
    // if the result is empty that means the user is not found.
    co_await getCon->async_execute(
        boost::mysql::with_params(
            "START TRANSACTION;"
            "SELECT hashed_password FROM users WHERE username = "
            "{0} OR email = {0};"
            "SELECT public_id,role,username,profile_picture FROM users "
            "WHERE username = {0} OR email = {0};",
            user.emailOrUsername),
        result, diag, boost::asio::redirect_error(ec));
    // getCon.return_without_reset();

    if (ec) {
      std::cerr << ec.what() << " from 1" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    if (result.rows<1>().empty())
      co_return std::unexpected(makeErrorCode(AppError::User_Not_Found));

    std::string hashedPassword{std::get<0>(result.rows<1>()[0])};

    if (crypto_pwhash_str_verify(hashedPassword.c_str(), user.password.c_str(),
                                 user.password.size()) != 0)
      co_return std::unexpected(makeErrorCode(AppError::Invalid_Credentials));

    // if the login is sucessfull
    // create a jwt token
    // and and accesstoken
    auto sendUserLogin = result.rows<2>()[0];
    auto public_id = sendUserLogin.public_id;

    boost::mysql::static_results<std::tuple<>, std::tuple<>,
                                 std::tuple<std::string>, std::tuple<>>
        result2;

    co_await getCon->async_execute(boost::mysql::with_params(
                                       R"(
                                                    DELETE rt FROM users_refresh_token rt 
                                                    INNER JOIN users ON users.user_id = rt.refresh_token_user_id 
                                                    WHERE public_id = {0};
                                                    INSERT INTO users_refresh_token(refresh_token_user_id,expires_at) 
                                                    VALUES((SELECT user_id FROM users WHERE public_id = {0}),
                                                    current_timestamp() + INTERVAL {1} SECOND);
                                                    SELECT refresh_token FROM users_refresh_token WHERE 
                                                    refresh_token_id = LAST_INSERT_ID(); 
                                                    COMMIT;
                                                )",
                                       public_id, REFRESHTOKENEXPIRATIONTIME),
                                   result2, diag,
                                   boost::asio::redirect_error(ec));
    getCon.return_without_reset();

    if (ec) {
      std::cerr << ec.what() << "from 2" << std::endl;
      if (!diag.server_message().empty()) {
        std::cerr << diag.server_message() << std::endl;
      }

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    auto accessToken = Security::createJwt(
        boost::json::object{{"public_id", sendUserLogin.public_id},
                            {"role", sendUserLogin.role}},
        std::chrono::system_clock::now() +
            std::chrono::seconds(ACCESSTOKENEXPIRATIONTIME),
        SECRETEKEY);

    auto refreshToken = std::get<0>(result2.rows<2>()[0]);

    // insert the refresh token inside the db.
    // accesstoken is the name of the db.

    LoginSuccessFullSchema loginSuccessFullSchema;
    loginSuccessFullSchema.userInformation = sendUserLogin;
    loginSuccessFullSchema.accessToken = accessToken;
    loginSuccessFullSchema.refreshToken = refreshToken;

    co_return loginSuccessFullSchema;
  }
};