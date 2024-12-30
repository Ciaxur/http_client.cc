#include "http/http_client.h"
#include "http/scope.h"
#include <fmt/format.h>

int main() {
  // Instead of manually calling init and cleanup, we can leverage
  // OOP's destructor and have it clean up for us. This way early returns
  // without calling cleanup logic can be done cleanly.
  //
  // The first parameter is a function that will be invoked within
  // the constructor and the second parameter is a function that
  // will be invoked within the destructor.
  http::utility::Scoped http_scope(http::init, http::cleanup);

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

  return 0;
}

