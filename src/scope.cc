#include "http/scope.h"


namespace http::utility {
  Scoped::Scoped(std::optional<std::function<void(void)>> init_fn, std::optional<std::function<void(void)>> de_init_fn) {
    if (init_fn.has_value()) init_fn.value()();
    if (de_init_fn.has_value()) this->de_init_fn = de_init_fn;
  }

  Scoped::~Scoped() {
    if (this->de_init_fn.has_value()) this->de_init_fn.value()();
  }
}

