#include <iostream>
#include <utility>

#include "HttpsRequest.hpp"
#include "InputDirectoryConfig.hpp"
#include "Utilities.ipp"

namespace
{
    std::optional<std::string> GetCookie()
    {
        const auto sessionFile =
            std::format("{}/.adventofcode.session", config::GetInputFilePath());
        if(const auto sessions = util::ParseToContainer(sessionFile); !sessions.empty())
        {
            return std::format("session={}", sessions.front());
        }

        return {};
    }

    // https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
    // ReSharper disable CppParameterMayBeConst
    // ReSharper disable IdentifierTypo
    // ReSharper disable CppCStyleCast
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    // ReSharper restore CppCStyleCast
    // ReSharper restore IdentifierTypo
    // ReSharper restore CppParameterMayBeConst
}

HttpsRequest::HttpsRequest()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // TODO: Could probably make sure we don't already have that file...
    mCurl = curl_easy_init();

    // Disable progress bar
    curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, 1L);

    // Read contents into mReadBuffer.
    curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, &mReadBuffer);

    // Include user agent information in the header
    // https://www.reddit.com/r/adventofcode/wiki/faqs/automation/
    const auto userAgent =
        "https://github.com/jonathondgebhardt/aoc-cli-cpp by jonathon.gebhardt@gmail.com";
    curl_easy_setopt(mCurl, CURLOPT_USERAGENT, userAgent);

    if(const auto cookie = GetCookie())
    {
        curl_easy_setopt(mCurl, CURLOPT_COOKIE, cookie->c_str());
    }
    else
    {
        std::println(std::cerr, "Could not load session file");
    }
}

HttpsRequest::HttpsRequest(HttpsRequest&& other) noexcept
{
    mCurl = std::exchange(other.mCurl, nullptr);
    mReadBuffer = std::exchange(other.mReadBuffer, {});
}

HttpsRequest::~HttpsRequest()
{
    if(mCurl)
    {
        curl_easy_cleanup(mCurl);
    }

    // TODO: CURL says this should be called once per application.
    curl_global_cleanup();
}

HttpsRequest& HttpsRequest::operator=(HttpsRequest&& other) noexcept
{
    if(mCurl)
    {
        curl_easy_cleanup(mCurl);
    }

    // TODO: Don't repeat yourself.
    mCurl = std::exchange(other.mCurl, nullptr);
    mReadBuffer = std::exchange(other.mReadBuffer, {});

    return *this;
}

void HttpsRequest::setUrl(const std::string_view url) const
{
    if(curl_easy_setopt(mCurl, CURLOPT_URL, url.data()) == CURLE_OUT_OF_MEMORY)
    {
        throw std::runtime_error("curl: out of memory");
    }
}

void HttpsRequest::setContentType(const std::string_view type) const
{
    curl_slist* list = nullptr;
    const auto content = std::format("Content-Type: {}", type);
    list = curl_slist_append(list, content.c_str());

    if(curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, list) == CURLE_UNKNOWN_OPTION)
    {
        throw std::runtime_error("curl: unknown option");
    }
}

std::optional<std::string> HttpsRequest::operator()() const
{
    if(mCurl)
    {
        if(curl_easy_perform(mCurl) == CURLE_OK)
        {
            return mReadBuffer;
        }

        std::println(std::cerr, "Could not perform HTTPS request");
    }
    else
    {
        std::println(std::cerr, "Could not initialize CURL environment");
    }

    return {};
}
