// Minimal self-registering test registry (no external libraries).
#pragma once
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace mini_test {

struct Case {
  const char *name;
  void (*fn)();
};

inline std::vector<Case> &registry() {
  static std::vector<Case> cases;
  return cases;
}

inline int &failures() {
  static int n = 0;
  return n;
}

struct Registrar {
  Registrar(const char *name, void (*fn)()) { registry().push_back({name, fn}); }
};

inline int run_all() {
  for (const Case &c : registry()) {
    int before = failures();
    c.fn();
    std::printf("%s %s\n", failures() == before ? "ok  " : "FAIL", c.name);
  }
  std::printf("%zu tests, %d failed assertions\n", registry().size(), failures());
  return failures() == 0 ? 0 : 1;
}

}  // namespace mini_test

#define TEST(name)                                                    \
  static void name();                                                 \
  static mini_test::Registrar registrar_##name(#name, &name);         \
  static void name()

#define CHECK(cond)                                                              \
  do {                                                                           \
    if (!(cond)) {                                                               \
      std::printf("  %s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);     \
      ++mini_test::failures();                                                   \
    }                                                                            \
  } while (0)

#define CHECK_EQ(expected, actual)                                                        \
  do {                                                                                    \
    auto e_ = (expected);                                                                 \
    auto a_ = (actual);                                                                   \
    if (!(e_ == a_)) {                                                                    \
      std::printf("  %s:%d: CHECK_EQ(%s, %s) failed: expected %lld, actual %lld\n",       \
                  __FILE__, __LINE__, #expected, #actual, (long long) e_, (long long) a_); \
      ++mini_test::failures();                                                            \
    }                                                                                     \
  } while (0)
