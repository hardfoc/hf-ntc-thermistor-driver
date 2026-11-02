/**
 * @file test_harness.hpp
 * @brief Minimal dependency-free test registry and check macros for the host unit tests.
 */
#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace ntc_test {

struct TestCase {
  const char* name;
  std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> tests;
  return tests;
}

inline int& failureCount() {
  static int failures = 0;
  return failures;
}

struct Registrar {
  Registrar(const char* name, std::function<void()> fn) {
    registry().push_back({name, std::move(fn)});
  }
};

inline void reportFailure(const char* file, int line, const std::string& what) {
  ++failureCount();
  std::printf("    FAIL %s:%d: %s\n", file, line, what.c_str());
}

} // namespace ntc_test

#define NTC_TEST_CONCAT2(a, b) a##b
#define NTC_TEST_CONCAT(a, b) NTC_TEST_CONCAT2(a, b)

/// Define a test case.
#define TEST_CASE(name)                                                                            \
  static void NTC_TEST_CONCAT(test_fn_, __LINE__)();                                               \
  static const ::ntc_test::Registrar NTC_TEST_CONCAT(test_reg_, __LINE__)(                     \
      name, &NTC_TEST_CONCAT(test_fn_, __LINE__));                                                 \
  static void NTC_TEST_CONCAT(test_fn_, __LINE__)()

/// Non-fatal boolean check.
#define CHECK(cond)                                                                                \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      ::ntc_test::reportFailure(__FILE__, __LINE__, #cond);                                    \
    }                                                                                              \
  } while (0)

/// Non-fatal equality check with values printed as unsigned long long.
#define CHECK_EQ(a, b)                                                                             \
  do {                                                                                             \
    const auto ntc_test_a = (a);                                                                   \
    const auto ntc_test_b = (b);                                                                   \
    if (!(ntc_test_a == ntc_test_b)) {                                                             \
      ::ntc_test::reportFailure(                                                               \
          __FILE__, __LINE__,                                                                      \
          std::string(#a " == " #b " (") +                                                         \
              std::to_string(static_cast<unsigned long long>(ntc_test_a)) + " vs " +               \
              std::to_string(static_cast<unsigned long long>(ntc_test_b)) + ")");                  \
    }                                                                                              \
  } while (0)

/// Fatal check: stop the current test case.
#define REQUIRE(cond)                                                                              \
  do {                                                                                             \
    if (!(cond)) {                                                                                 \
      ::ntc_test::reportFailure(__FILE__, __LINE__, #cond);                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/// Non-fatal floating-point check: |a - b| <= tol, values printed with two decimals.
#define CHECK_NEAR(a, b, tol)                                                                      \
  do {                                                                                             \
    const double ntc_test_a = static_cast<double>(a);                                              \
    const double ntc_test_b = static_cast<double>(b);                                              \
    if (!(ntc_test_a - ntc_test_b <= (tol) && ntc_test_b - ntc_test_a <= (tol))) {                 \
      char ntc_test_buf[96];                                                                       \
      std::snprintf(ntc_test_buf, sizeof ntc_test_buf, " (%.3f vs %.3f)", ntc_test_a, ntc_test_b); \
      ::ntc_test::reportFailure(__FILE__, __LINE__,                                                \
                                std::string(#a " ~= " #b) + ntc_test_buf);                         \
    }                                                                                              \
  } while (0)
