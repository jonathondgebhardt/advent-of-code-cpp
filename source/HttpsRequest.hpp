#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <curl/curl.h>

class https_request
{
public:
  https_request();
  https_request(const https_request&) = delete;
  https_request(https_request&& other) noexcept;

  ~https_request();

  auto operator=(const https_request&) -> https_request& = delete;
  auto operator=(https_request&& other) noexcept -> https_request&;

  void set_url(std::string_view url) const;
  void set_content_type(std::string_view type) const;

  auto operator()() const -> std::optional<std::string>;

private:
  CURL* m_curl = nullptr;
  std::string m_read_buffer;
};
