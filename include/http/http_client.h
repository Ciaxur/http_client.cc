#pragma once

#include <curl/curl.h>
#include <cstdint>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

// NOTE: https://curl.se/libcurl

#define HTTP_CLIENT_DEFAULT_USER_AGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36"
#define HTTP_CLIENT_DEFAULT_COOKIES_PATH "cookies.txt"

namespace http {
  size_t recv_headers(char* buffer, size_t size, size_t nitems, std::unordered_map<std::string, std::string>& user_data);
  size_t writefunc(void *ptr, size_t size, size_t nmemb, std::stringstream* ss);

  /**
   * Globally initializes the CURL library.
   */
  void init();

  /**
   * Globally cleans up the CURL library.
   */
  void cleanup();

  /**
   * Parses encoded parameters from a given URL.
   *
   * @param url URL to extract parameters from.
   *
   * @returns map containing the key and value of extracted parameters.
   */
  std::map<std::string, std::string> parse_url_params(const std::string& url);

  /**
  * Curl result structure, which includes the resulting status code
  * and body.
  */
  struct CurlResult {
    CURLcode    code;
    std::string output;
    std::string error;
    std::unordered_map<std::string, std::string> response_headers;
  };

  /**
  * URL parameter structure.
  */
  struct UrlParam {
    const char* key;
    const char* value;
  };


  class HttpClient {
  private:
    std::stringstream _body_stream;
    std::string _url;
    std::list<UrlParam> _params;
    std::unordered_map<std::string, std::string> _resp_header_map;

    CURL *_curl;
    curl_slist *_headers;

  private:
    HttpClient();

  public:
    ~HttpClient();

    /**
    * Initial entrypoint to start building an HttpClient instance.
    *
    * @returns HttpClient instance.
    */
    static HttpClient build() {
      return HttpClient();
    }

    /**
    * Configures the HttpClient instance to be a GET request on
    * the given URL.
    *
    * @param url url to invoke GET request on.
    * @returns HttpClient instance.
    */
    HttpClient& get(const char* url);

    /**
    * Configures the HttpClient instance's URL to the given URL.
    *
    * @param url url to invoke request on.
    * @returns HttpClient instance.
    */
    HttpClient& set_url(const char* url);

    /**
     * Sets a Header onto the HttpClient instance.
     *
     * @param header_entry Header to set.
     * @returns HttpClient instance.
     */
    HttpClient& set_header(const char *header_entry);

    /**
     * Sets the raw body to include with the request.
     *
     * @param body String containing raw body request.
     * @returns HttpClient instance.
     */
    HttpClient& set_raw_body(const std::string body);

    /**
    * Sets authority header.
    *
    * @param auth_str authority header value.
    * @returns HttpClient instance.
    */
    HttpClient& set_authority_header(const char* auth_str);

    /**
    * Sets default headers onto HttpClient instance. Default headers
    * define a browser as the User-Agent and Accept heaers.
    *
    * @returns HttpClient instance.
    */
    HttpClient& set_default_headers();

    /**
    * Sets the Cookie filepath to save and retrieve cookies from.
    *
    * @param path Path to the cookie file.
    * @returns HttpClient instance.
    */
    HttpClient& set_cookie_filepath(const char* path);

    /**
    * Sets the verbosity of the HttpClient instance.
    *
    * @param value Verbosity value.
    * @returns HttpClient instance.
    */
    HttpClient& set_verbosity(uint64_t value);

    /**
    * Adds a URL Parameter to the instance.
    *
    * @param param UrlParam instance used to add onto the URL.
    * @returns HttpClient instance.
    */
    HttpClient& add_url_param(const UrlParam& param);

    /**
    * Executes the built HttpClient structure, returning the result.
    *
    * @return CurlResult instance.
    */
    CurlResult execute();
  };
}

