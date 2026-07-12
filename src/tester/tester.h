#ifndef TESTER_H
#define TESTER_H

#include <string>
#include <vector>

bool runTests(const std::string& executablePath,
              const std::string& testCasesPath,
              std::vector<std::string>& failedTests,
              std::string& errorMessage,
              int timeoutSeconds = 5);

#endif // TESTER_H
