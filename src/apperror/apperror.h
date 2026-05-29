#pragma once

#include <system_error>
#include <type_traits>

enum class AppError {
  success = 0,
  Email_Already_In_Use,
  Username_Already_In_use,
  Invalid_Credentials,
  Password_Not_Good,
  Reading_Request_Failed,
  Writing_Response_Failed,
  Convert_Body_To_Object_Failed,
  No_Database_Connection,
  Server_Failed,
  User_Not_Found,
  AccessToken_Expired,
  Invalid_AcessToken,
  Invalid_RefreshToken,
  RefreshToken_Expired,
  Unauthorize_Access,
  Invalid_Path_Parameter,
  Category_Not_Found,
  Post_Not_Found,
  Comment_Not_Found,
  Illegal_post_Operation,
  Illegal_Comment_Operation,
  UnKnown
};

inline const std::error_category &appErrorCategory() {
  class AppErrorCategory : public std::error_category {
    const char *name() const noexcept override { return "AppErrorCategory"; }

    std::string message(int e) const override {
      switch (static_cast<AppError>(e)) {
      case AppError::success:
        return "successfull";
      case AppError::Email_Already_In_Use:
        return "email already in use";
      case AppError::Username_Already_In_use:
        return "username already in use";
      case AppError::Invalid_Credentials:
        return "invalid credentials";
      case AppError::Password_Not_Good:
        return "password could not be processed";
      case AppError::Reading_Request_Failed:
        return "could not read request";
      case AppError::Writing_Response_Failed:
        return "could not write response";
      case AppError::Convert_Body_To_Object_Failed:
        return "could not convert body to object";
      case AppError::No_Database_Connection:
        return "could not connect to database";
      case AppError::Server_Failed:
        return "internal server error";
      case AppError::User_Not_Found:
        return "user not found";
      case AppError::Category_Not_Found:
        return "category not category";
      case AppError::AccessToken_Expired:
        return "access token has expired";
      case AppError::Invalid_AcessToken:
        return "invalid access token";
      case AppError::RefreshToken_Expired:
        return "refresh token has expired";
      case AppError::Invalid_RefreshToken:
        return "invalid refresh token";
      case AppError::Unauthorize_Access:
        return "unauthorize access, login in to gain access";
      case AppError::Invalid_Path_Parameter:
        return "invalid path parameter";
      case AppError::Illegal_post_Operation:
        return "operation only perminted on owners post";
      case AppError::Illegal_Comment_Operation:
        return "operation only perminted on owners comment";
      case AppError::Post_Not_Found:
        return "post not found";
      case AppError::Comment_Not_Found:
        return "comment not found";
      default:
        return "unknown error";
      }
    }
  };

  const static AppErrorCategory appErrorCat;
  return appErrorCat;
}

inline std::error_code makeErrorCode(AppError e) {
  return std::error_code(static_cast<int>(e), appErrorCategory());
}

namespace std {
template <> struct is_error_code_enum<AppError> : true_type {};
} // namespace std