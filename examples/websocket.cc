#include "http/scope.h"
#include <fmt/base.h>
#include <http/http_client.h>

int main() {
  http::utility::Scoped http_scope(http::init, http::cleanup);
  http::HttpClient client = http::HttpClient::build();

  // On receive stream callback.
  //
  // NOTE: Make sure to reset stringstream after consumption, otherwise
  // the internal stream cursor is set to -1 and thus won't allow writes.
  // This'll result in subsequent received data to not get buffered.
  bool wrote_data = false;
  std::function<void(http::WriteBuffer*)> on_recv_cb = [&](http::WriteBuffer* wb) {
    std::string buffer;
    buffer.reserve(2048);
    std::getline(wb->ss, buffer);
    fmt::println("recv -> '{}'", buffer);

    // Reset stringstream flags.
    wb->ss.clear();

    if (wrote_data) {
      client.stream_close();
      return;
    };

    wrote_data = true;
    std::string resp_buffer = "hello from client";
    fmt::println("send -> '{}'", resp_buffer);
    client.stream_write(resp_buffer);
  };

  http::CurlResult res = client
    .set_url("wss://echo.websocket.org")
    .set_on_recv_callback(&on_recv_cb)
    .stream()
    .execute();

  return 0;
}
