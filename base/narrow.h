// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

// ort_extensions::narrow() is like gsl::narrow() but it is also available when exceptions are disabled.

#if !defined(OCOS_NO_EXCEPTIONS)

#include "gsl/narrow"

namespace ort_extensions {
using gsl::narrow;
}  // namespace ort_extensions

#else  // !defined(OCOS_NO_EXCEPTIONS)

#include <cstdio>     // std::fprintf
#include <exception>  // std::terminate
#include <type_traits>

#include "gsl/util"  // gsl::narrow_cast

namespace ort_extensions {

namespace detail {
[[noreturn]] inline void OnNarrowingError() noexcept {
  std::fprintf(stderr, "%s", "narrowing error\n");
  std::terminate();
}
}  // namespace detail

// This implementation of ort_extensions::narrow was copied and adapted from:
// https://github.com/microsoft/onnxruntime/blob/77b455b969db15d911cd6de5009f3a30fb42c531/include/onnxruntime/core/common/narrow.h
// which was copied and adapted from
// https://github.com/microsoft/GSL/blob/a3534567187d2edc428efd3f13466ff75fe5805c/include/gsl/narrow

// narrow() : a checked version of narrow_cast() that terminates if the cast changed the value
template <class T, class U, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
// clang-format off
GSL_SUPPRESS(type.1) // NO-FORMAT: attribute
                      // clang-format on
    constexpr T narrow(U u) noexcept {
  constexpr const bool is_different_signedness =
      (std::is_signed<T>::value != std::is_signed<U>::value);

  // clang-format off
GSL_SUPPRESS(es.103) // NO-FORMAT: attribute // don't overflow
GSL_SUPPRESS(es.104) // NO-FORMAT: attribute // don't underflow
GSL_SUPPRESS(p.2) // NO-FORMAT: attribute // don't rely on undefined behavior
  // clang-format on
  const T t = gsl::narrow_cast<T>(u);  // While this is technically undefined behavior in some cases (i.e., if the source value is of floating-point type
                                       // and cannot fit into the destination integral type), the resultant behavior is benign on the platforms
                                       // that we target (i.e., no hardware trap representations are hit).

  if (static_cast<U>(t) != u || (is_different_signedness && ((t < T{}) != (u < U{})))) {
    detail::OnNarrowingError();
  }

  return t;
}

template <class T, class U, typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
// clang-format off
GSL_SUPPRESS(type.1) // NO-FORMAT: attribute
                      // clang-format on
    constexpr T narrow(U u) noexcept {
  const T t = gsl::narrow_cast<T>(u);

  if (static_cast<U>(t) != u) {
    detail::OnNarrowingError();
  }

  return t;
}

}  // namespace ort_extensions

#endif  // !defined(OCOS_NO_EXCEPTIONS)
