// Runs every registered test.
//   ./build/run_tests            run everything
//   ./build/run_tests Castling   run only tests whose name contains "Castling"
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include "TestFramework.h"

#ifndef _WIN32
#include <unistd.h>
#endif

// If the engine crashes (for example a bad pointer), say which test was running
static char runningTest[256] = "";

#ifndef _WIN32
static void onCrash(int) {
    const char text[] = "\n  [CRASH] the program crashed while running: ";
    (void)!write(STDOUT_FILENO, text, sizeof(text) - 1);
    (void)!write(STDOUT_FILENO, runningTest, std::strlen(runningTest));
    (void)!write(STDOUT_FILENO, "\n\nTests stopped early because of the crash.\n", 43);
    _exit(2);
}
#endif

int main(int argc, char** argv) {
#ifndef _WIN32
    std::signal(SIGSEGV, onCrash);
    std::signal(SIGABRT, onCrash);
    std::signal(SIGFPE, onCrash);
#endif

    const std::string filter = argc > 1 ? argv[1] : "";
    std::vector<testing::TestCase> tests = testing::registry();
    std::stable_sort(tests.begin(), tests.end(),
                     [](const testing::TestCase& a, const testing::TestCase& b) { return a.suite < b.suite; });

    int passed = 0;
    int failed = 0;
    std::string lastSuite;
    for (const testing::TestCase& t : tests) {
        const std::string fullName = t.suite + "." + t.name;
        if (!filter.empty() && fullName.find(filter) == std::string::npos) continue;
        if (t.suite != lastSuite) {
            std::cout << "\n" << t.suite << "\n" << std::flush;
            lastSuite = t.suite;
        }

        std::strncpy(runningTest, fullName.c_str(), sizeof(runningTest) - 1);
        testing::failureCount() = 0;
        testing::failureText().clear();
        const auto start = std::chrono::steady_clock::now();
        try {
            t.body();
        } catch (const std::exception& e) {
            testing::fail(__FILE__, __LINE__, std::string("exception: ") + e.what());
        }
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

        if (testing::failureCount() == 0) {
            std::cout << "  [PASS] " << t.name;
            if (ms >= 50) std::cout << "  (" << ms << " ms)";
            std::cout << "\n";
            passed++;
        } else {
            std::cout << "  [FAIL] " << t.name << "\n" << testing::failureText();
            failed++;
        }
        std::cout << std::flush;
    }

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
