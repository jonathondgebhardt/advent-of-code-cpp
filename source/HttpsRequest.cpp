#include <iostream>
#include <utility>

#include "HttpsRequest.hpp"

#include "InputDirectoryConfig.hpp"
#include "Utilities.hpp"

namespace
{
auto get_cookie() -> std::optional<std::string>
{
  const auto session_file =
      std::format("{}/.adventofcode.session", config::input_file_path);
  if (const auto sessions = util::parse_to_container(session_file);
      !sessions.empty())
  {
    return std::format("session={}", sessions.front());
  }

  return {};
}

// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
auto write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    -> size_t
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

}  // namespace

https_request::https_request()
    : m_curl(curl_easy_init())
{
  curl_global_init(CURL_GLOBAL_DEFAULT);

  // TODO: Could probably make sure we don't already have that file...

  // Disable progress bar
  curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1L);

  // Read contents into mReadBuffer.
  curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_read_buffer);

  // Include user agent information in the header
  // https://www.reddit.com/r/adventofcode/wiki/faqs/automation/
  const auto *const user_agent =
      "https://github.com/jonathondgebhardt/aoc-cli-cpp by "
      "jonathon.gebhardt@gmail.com";
  curl_easy_setopt(m_curl, CURLOPT_USERAGENT, user_agent);

  if (const auto cookie = get_cookie()) {
    curl_easy_setopt(m_curl, CURLOPT_COOKIE, cookie->c_str());
  } else {
    std::println(std::cerr, "Could not load session file");
  }
}

https_request::https_request(https_request&& other) noexcept
{
  m_curl = std::exchange(other.m_curl, nullptr);
  m_read_buffer = std::exchange(other.m_read_buffer, {});
}

https_request::~https_request()
{
  if (m_curl != nullptr) {
    curl_easy_cleanup(m_curl);
  }

  // TODO: CURL says this should be called once per application.
  curl_global_cleanup();
}

auto https_request::operator=(https_request&& other) noexcept -> https_request&
{
  if (m_curl != nullptr) {
    curl_easy_cleanup(m_curl);
  }

  // TODO: Don't repeat yourself.
  m_curl = std::exchange(other.m_curl, nullptr);
  m_read_buffer = std::exchange(other.m_read_buffer, {});

  return *this;
}

void https_request::set_url(const std::string_view url) const
{
  curl_easy_setopt(m_curl, CURLOPT_URL, url.data());
}

void https_request::set_content_type(const std::string_view type) const
{
  curl_slist* list = nullptr;
  const auto content = std::format("Content-Type: {}", type);
  list = curl_slist_append(list, content.c_str());
  curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, list);
}

auto https_request::operator()() const -> std::optional<std::string>
{
  if (m_curl != nullptr) {
    if (curl_easy_perform(m_curl) == CURLE_OK) {
      return m_read_buffer;
    }

    std::println(std::cerr, "Could not perform HTTPS request");
  } else {
    std::println(std::cerr, "Could not initialize CURL environment");
  }

  return {};
}
