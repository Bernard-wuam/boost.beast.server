#pragma once
#include "apperror/apperror.h"
#include <expected>
#include <regex>
#include <string>
#include <system_error>

class RegexPath {
public:
  RegexPath(const std::string &regex) : m_regex(regex) {};
  constexpr bool isMatched(const std::string &path) {
    m_path = path;
    return std::regex_match(path.data(), m_regex);
  };

  std::expected<std::string, std::error_code>
  extraPathId(const std::regex &regex) const {
    std::smatch smatch;
    if (std::regex_search(m_path, smatch, regex)) {
      if (smatch.size() > 1)
        return smatch[0];
    };

    return std::unexpected(makeErrorCode(AppError::Invalid_Path_Parameter));
  }
  void setRegexUrl(const std::string &regex) { m_regex = regex; }

private:
  std::regex m_regex;
  std::string m_path;
};