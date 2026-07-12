#ifndef AGENT_H
#define AGENT_H

#include <string>
#include <vector>

struct IterationResult {
    int iteration;
    std::string code;
    bool compiled;
    std::string compilerError;
    bool testsPassed;
    std::vector<std::string> testResults;
    std::vector<std::string> failedTests;
    std::string diffFromPrevious;
};

class Agent {
public:
    Agent(int maxIterations = 5, const std::string& testCasesPath = "tests/test_cases.json");
    void run(const std::string& problemDescription);
    void printFinalReport();

private:
    int maxIterations;
    std::string testCasesPath;
    std::vector<IterationResult> history;
    std::string apiKey;

    std::string callGemini(const std::string& prompt);
    std::string generateDiff(const std::string& oldCode, const std::string& newCode);
};

#endif
