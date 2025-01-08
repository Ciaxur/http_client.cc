#include "http/http_client.h"
#include <curl/easy.h>
#include <fmt/format.h>
#include <filesystem>
#include <fstream>

namespace http {
  void init() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }

  void cleanup() {
    curl_global_cleanup();
  }


  size_t recv_headers(char* buffer, size_t size, size_t nitems, std::unordered_map<std::string, std::string>& header_map) {
    size_t total_size = size * nitems;
    const std::string buffer_str(buffer, total_size);

    // Ignore empty headers.
    if (buffer_str == "\r\n" || buffer_str == "\n") {
        return total_size;
    }

    // Find the colon within the buffer header, as that's how headers are
    // constructed.
    size_t colon_pos = buffer_str.find(":");
    if (colon_pos != std::string::npos) {
      const std::string key = buffer_str.substr(0, colon_pos);
      std::string value = buffer_str.substr(colon_pos + 1);

      // Trim any whitespace.
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);

      value.erase(0, value.find_first_not_of("\r\n"));
      value.erase(value.find_last_not_of("\r\n") + 1);

      header_map.insert({ key, value });
    }

    return total_size;
  }

  size_t writefunc(void *ptr, size_t size, size_t nmemb, std::stringstream* ss) {
    ss->write(static_cast<const char*>(ptr), nmemb);
    return size*nmemb;
  }

  std::map<std::string, std::string> parse_url_params(const std::string& url) {
    std::map<std::string, std::string> params;
    size_t query_start = url.find('?');

    if (query_start != std::string::npos) {
        std::string query = url.substr(query_start + 1);
        std::istringstream query_stream(query);
        std::string key_value;

        while (std::getline(query_stream, key_value, '&')) {
            size_t equal_pos = key_value.find('=');
            if (equal_pos != std::string::npos) {
                std::string key = key_value.substr(0, equal_pos);
                std::string value = key_value.substr(equal_pos + 1);
                params[key] = value;
            }
        }
    }

    return params;
  }

  std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
      std::string::value_type c = (*i);

      // Keep alphanumeric and other accepted characters intact
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        escaped << c;
        continue;
      }

      // Any other characters are percent-encoded
      escaped << std::uppercase;
      escaped << '%' << std::setw(2) << int((unsigned char)c);
      escaped << std::nouppercase;
    }

    return escaped.str();
  }

  HttpClient::HttpClient(): _curl(nullptr), _headers(nullptr), _body_stream(), _resp_header_map() {
    this->_curl = curl_easy_init();

    // Default options.
    // Enable SSL.
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 1L);

    // Cache CA certificate bundle in memory for 1 week.
    curl_easy_setopt(_curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

    // "-L" flag. Follows location of endpoint if it moved.
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Store output into stream buffer.
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &this->_body_stream);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, writefunc);

    // Store recieved headers.
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, &this->_resp_header_map);
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, recv_headers);
  }

  HttpClient::~HttpClient() {
    curl_slist_free_all(this->_headers);
    curl_easy_cleanup(this->_curl);

    this->_curl = nullptr;
    this->_headers = nullptr;
  }

  HttpClient& HttpClient::get(const char* url) {
    this->_url = url;

    // "-G" flag.
    curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
    return *this;
  }

  HttpClient&  HttpClient::set_header(const char* header_entry) {
    this->_headers = curl_slist_append(this->_headers, header_entry);
    return *this;
  }

  HttpClient& HttpClient::set_url(const char* url) {
    this->_url = url;
    return *this;
  }

  HttpClient& HttpClient::set_raw_body(const std::string body) {
    curl_easy_setopt(this->_curl, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(this->_curl,  CURLOPT_COPYPOSTFIELDS, body.c_str());
    return *this;
  }

  HttpClient& HttpClient::set_authority_header(const char* auth_str) {
    this->set_header(fmt::format("Authorization: Bearer {}", auth_str).c_str());
    return *this;
  }

  HttpClient& HttpClient::set_default_headers() {
    this->_headers = curl_slist_append(this->_headers, "user-agent: " HTTP_CLIENT_DEFAULT_USER_AGENT);
    this->_headers = curl_slist_append(this->_headers, "accept: */*");

    return *this;
  }

  HttpClient&  HttpClient::set_cookie_filepath(const char* path) {
    // "-c" and "-b" flags. Storing and reading cookies.
    curl_easy_setopt(this->_curl, CURLOPT_COOKIEJAR, path);
    curl_easy_setopt(this->_curl, CURLOPT_COOKIEFILE, path);

    // Make sure cookies file exists.
    if (!std::filesystem::exists(path)) {
      std::ofstream cookies_fs {path};
      cookies_fs.close();
    }

    return *this;
  }

  HttpClient&  HttpClient::set_verbosity(uint64_t value) {
    curl_easy_setopt(this->_curl, CURLOPT_VERBOSE, value);
    return *this;
  }

  HttpClient&  HttpClient::add_url_param(const UrlParam& param) {
    this->_params.push_back(param);
    return *this;
  }

  CurlResult HttpClient::execute() {
    // Set constructed headers.
    curl_easy_setopt(this->_curl, CURLOPT_HTTPHEADER, this->_headers);

    // Construct URL
    std::ostringstream url_ss;
    url_ss << this->_url;
    if (!this->_params.empty()) {
      // Include the first param.
      url_ss
        << "?"
        << url_encode(this->_params.front().key)
        << "="
        << url_encode(this->_params.front().value);
      this->_params.pop_front();

      // Add remaining params.
      for (const UrlParam& param : this->_params) {
        url_ss
          << "&"
          << url_encode(param.key)
          << "="
          << url_encode(param.value);
      }
    }
    curl_easy_setopt(_curl, CURLOPT_URL, url_ss.str().c_str());

    // Ship it!
    CURLcode res = curl_easy_perform(this->_curl);
    return {
      .code = res,
      .output = this->_body_stream.str(),
      .error = curl_easy_strerror(res),
      .response_headers = this->_resp_header_map,
    };
  }
}


