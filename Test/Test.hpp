#pragma once

#include <vector>
#include <cstdlib>
#include <iostream>
#include <assert.h>

namespace Testing {

    class Testable;

    static inline std::vector<Testable*> testCases;

    class Testable {
    public:
        Testable() {
            testCases.push_back(this);
        }

        bool RunTest() {
            Test();
            return !failed;
        }

        virtual ~Testable() = default;
    protected:
        virtual void Test() = 0;

        bool check(bool, std::string, std::string, std::string, int);

        bool failed = false;
    };

}

#define TEST(TestCase) \
class TestCase : public ::Testing::Testable { \
private: \
  void Test() override; \
}; \
namespace testing##TestCase { \
    ::TestCase test; \
} \
void TestCase::Test()

#define ASSERT(x) if (!Testing::Testable::check(x, __FILE__, #x, __FUNCTION__, __LINE__)) return;

#define RUN_ALL_TESTS \
{ \
  int failed = 0; \
  const int testCnt = ::Testing::testCases.size(); \
  for (auto t : ::Testing::testCases) \
    if (!t->RunTest()) \
        failed++;  \
  \
  std::cerr << "Test passed: " << testCnt - failed << "/" << testCnt << std::endl; \
  return failed; \
}
