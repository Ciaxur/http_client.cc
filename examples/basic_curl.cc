#include "http/http_client.h"
#include <fmt/format.h>

int main() {
  fmt::println("Initializing curl library");
  http::init();

  // Construct HTTP and issue request.
  http::CurlResult res = http::HttpClient::build()
    .get("https://ddg.gg")
    .set_default_headers()
    .set_verbosity(1)
    .set_cookie_filepath(HTTP_CLIENT_DEFAULT_COOKIES_PATH)
    .execute();

  fmt::println("Request result status code: {}", static_cast<int>(res.code));
  if (res.code != CURLE_OK) {
    fmt::println(
      "ERROR: HTTP request failed!\n"
      "  - Status code '{}'\n"
      "  - Body: {}\n",
      "  - Error: {}",
      static_cast<int>(res.code),
      res.output,
      res.error
    );
  }
  else {
    fmt::println("Response: {}", res.output);
  }


  fmt::println("Cleaning up curl library");
  http::cleanup();
  return 0;
}

