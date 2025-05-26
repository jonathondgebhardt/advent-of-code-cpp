#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <curl/curl.h>

class HttpsRequest
{
  public:
    HttpsRequest();
    HttpsRequest(const HttpsRequest&) = delete;
    HttpsRequest(HttpsRequest&& other) noexcept;

    ~HttpsRequest();

    HttpsRequest& operator=(const HttpsRequest&) = delete;
    HttpsRequest& operator=(HttpsRequest&& other) noexcept;

    void setUrl(std::string_view url) const;
    void setContentType(std::string_view type) const;

    std::optional<std::string> operator()() const;

  private:
    CURL* mCurl = nullptr;
    std::string mReadBuffer;
};
