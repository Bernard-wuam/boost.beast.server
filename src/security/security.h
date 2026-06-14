#pragma once
#include "apperror/apperror.h"
#include "posts/schemas/jwtuserinfoschema.h"
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/fields_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/object.hpp>
#include <chrono>
#include <expected>
#include <format>
#include <iterator>
#include <jwt-cpp/base.h>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/boost-json/traits.h>
#include <ranges>
#include <string_view>
#include <system_error>

namespace Security {

inline std::string createJwt(boost::json::object payload,
                             std::chrono::system_clock::time_point expireAt,
                             std::string_view secreteKey) {
  auto token = jwt::create<jwt::traits::boost_json>(jwt::default_clock{})
                   .set_type("JWS")
                   .set_issuer("boostbeastblog")
                   .set_payload_claim("userInfo", payload)
                   .set_expires_at(expireAt)
                   .sign(jwt::algorithm::hs256{secreteKey.data()});
  return token;
}

template <typename Body, typename Allocator>
inline std::expected<JwtUserInfoSchema, std::error_code>
verifyJwt(boost::beast::http::request<
              Body, boost::beast::http::basic_fields<Allocator>> &request,
          const std::string &secret) {
  std::string bearer = request[boost::beast::http::field::authorization];

  if (bearer.empty()) {
    request.keep_alive(false);
    return std::unexpected(makeErrorCode(AppError::Unauthorize_Access));
  }
  auto parts = bearer | std::ranges::views::split(' ');

  if (std::ranges::distance(parts) != 2)
    return std::unexpected(makeErrorCode(AppError::Unauthorize_Access));

  auto it = parts.begin();
  auto bearerIt = std::ranges::next(it, 1);

  std::string_view token(*bearerIt);
  std::error_code ec;
  try {

    auto decoded_token = jwt::decode<jwt::traits::boost_json>(token.data());
    auto verifier = jwt::verify<jwt::traits::boost_json>()
                        .with_issuer("boostbeastblog")
                        .allow_algorithm(jwt::algorithm::hs256{secret})
                        .expires_at_leeway(0);

    verifier.verify(decoded_token, ec);

    if (ec) {
      std::cout << ec.message() << std::endl;
      if (ec == jwt::error::token_verification_error::token_expired)
        return std::unexpected(makeErrorCode(AppError::AccessToken_Expired));
      return std::unexpected(makeErrorCode(AppError::Invalid_AcessToken));
    }

    auto val = decoded_token.get_payload_claim("userInfo").to_json();

    return boost::json::value_to<JwtUserInfoSchema>(val);
  } catch (const std::exception &ec) {
  };

  return std::unexpected(makeErrorCode(AppError::Invalid_AcessToken));
};

}; // namespace Security