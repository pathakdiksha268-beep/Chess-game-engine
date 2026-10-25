// A tiny test framework (no external libraries needed).
//
//   TEST(Suite, Name) { CHECK(condition); CHECK_EQ(actual, expected); }
//
// Each TEST registers itself; run_tests.cpp runs them all and prints PASS / FAIL.
#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* suite, const char* name, std::function<void()> body) {
        registry().push_back({suite, name, std::move(body)});
    }
};

// failures of the test that is running right now
inline int& failureCount() { static int n = 0; return n; }
inline std::string& failureText() { static std::string s; return s; }

inline void fail(const char* file, int line, const std::string& message) {
    failureCount()++;
    std::ostringstream out;
    out << "      " << file << ":" << line << "  " << message << "\n";
    failureText() += out.str();
}

template <typename A, typename B>
void checkEqual(const A& actual, const B& expected, const char* actualText, const char* file, int line) {
    if (!(actual == expected)) {
        std::ostringstream out;
        out << actualText << " is " << actual << ", expected " << expected;
        fail(file, line, out.str());
    }
}

}  // namespace testing

#define TEST(suite, name)                                                              \
    static void test_##suite##_##name();                                               \
    static testing::Registrar registrar_##suite##_##name(#suite, #name, test_##suite##_##name); \
    static void test_##suite##_##name()

#define CHECK(condition)                                                               \
    do {                                                                               \
        if (!(condition)) testing::fail(__FILE__, __LINE__, "CHECK failed: " #condition); \
    } while (0)

#define CHECK_EQ(actual, expected) \
    testing::checkEqual((actual), (expected), #actual, __FILE__, __LINE__)
