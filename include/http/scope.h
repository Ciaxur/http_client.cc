#pragma once



#include <functional>
#include <optional>
namespace http::utility {
  /**
   * Scopes the given init and de-init lambdas, allowing for out of scope
   * de-structing.
   */
  class Scoped {
    private:
       std::optional<std::function<void(void)>> de_init_fn;

    public:
      Scoped(
        std::optional<std::function<void(void)>> init_fn,
        std::optional<std::function<void(void)>> de_init_fn
      );
      ~Scoped();
  };
}

