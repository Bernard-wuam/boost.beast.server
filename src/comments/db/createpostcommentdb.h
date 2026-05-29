
#pragma once
#include "apperror/apperror.h"
#include "comments/schemas/createpostcommentschema.h"
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
#include <optional>
#include <sodium/crypto_pwhash.h>
#include <system_error>

class CreatePostCommentDB {
public:
  static boost::asio::awaitable<
      std::expected<boost::json::value, std::error_code>,
      boost::asio::strand<boost::asio::io_context::executor_type>>
  createPostCommentDB(boost::mysql::connection_pool &conn, const std::string &index,
                  const JwtUserInfoSchema &user,
                  const CreatePostCommentSchema &createPostCommentSchema) {
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
        firstResult;
    boost::mysql::static_results<std::tuple<std::optional<std::uint64_t>>>
        commentResult;
    boost::mysql::results result;

    std::optional<std::uint64_t> reply_to = std::nullopt;

    co_await getCon->async_execute(boost::mysql::with_params(
                                       R"(
                                                  START TRANSACTION;
                                                  SELECT posts.post_id FROM posts WHERE posts.public_id = {};
                                                )",
                                       index, user.public_id),
                                   firstResult, diag,
                                   boost::asio::redirect_error(ec));

    if (firstResult.rows<1>().empty())
      co_return std::unexpected(makeErrorCode(AppError::Post_Not_Found));

    auto post_id = std::get<0>(firstResult.rows<1>()[0]);

    if (createPostCommentSchema.reply_to.has_value()) {
      co_await getCon->async_execute(boost::mysql::with_params(
                                         R"(
                                                    SELECT comments.comment_id FROM comments 
                                                    WHERE comments.comment_public_id = {}
                                                    AND comments.post_id = {};
                                                  )",
                                         createPostCommentSchema.reply_to.value(),post_id),
                                     commentResult, diag,
                                     boost::asio::redirect_error(ec));
      if (ec) {
        std::cout << "from getting the comments_id" << std::endl;
        std::cerr << ec.what() << std::endl;
        if (!diag.server_message().empty())
          std::cerr << diag.server_message() << std::endl;

        co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
      }
      if (commentResult.rows<0>().empty()) {
        co_return std::unexpected(makeErrorCode(AppError::Comment_Not_Found));
      }
      reply_to = std::get<0>(commentResult.rows<0>()[0]);
    }

    co_await getCon->async_execute(
        boost::mysql::with_params(
            R"(
                      INSERT INTO comments(post_id,user_id,reply_to,content) 
                      VALUES
                      (
                        {},
                        (SELECT users.user_id from users WHERE users.public_id = {}),
                        {},
                        {}
                      );
                      COMMIT;
                    )",
           post_id, user.public_id, reply_to, createPostCommentSchema.content),
        result, diag, boost::asio::redirect_error(ec));
    getCon.return_without_reset();

    if (ec) {
      std::cout << "from get post" << std::endl;
      std::cerr << ec.what() << std::endl;
      if (!diag.server_message().empty())
        std::cerr << diag.server_message() << std::endl;

      co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    }

    // if (result.rows().at(0).empty() || result.rows().at(0).at(0).is_null())
    //   co_return std::unexpected(makeErrorCode(AppError::Post_Not_Found));

    // auto val = result.rows().at(0).at(0).as_string();

    // auto obj = boost::json::parse(val, ec);

    // if (ec) {
    //   co_return std::unexpected(makeErrorCode(AppError::Server_Failed));
    // }

    boost::json::value obj(boost::json::object{{"result", "inserted"}});

    co_return obj;
  }
};