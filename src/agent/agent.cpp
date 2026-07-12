#include "agent.h"
#include "../report/report.h"
#include "../llm/gemini_client.h"
#include "../compiler/compiler.h"
#include "../tester/tester.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

#ifdef _WIN32
static const string EXECUTABLE_PATH = "temp\\program.exe";
static const string EXECUTABLE_PATH_FORWARD = "temp/program.exe";
#else
static const string EXECUTABLE_PATH = "./temp/program";
static const string EXECUTABLE_PATH_FORWARD = "./temp/program";
#endif

Agent::Agent(int maxIter, const std::string& testsPath)
    : maxIterations(maxIter), testCasesPath(testsPath) {
    const char* key = getenv("GEMINI_API_KEY");
    apiKey = key ? key : "";
}

string Agent::callGemini(const string& prompt) {
    if (apiKey.empty()) {
        cerr << "GEMINI_API_KEY not set. Cannot call LLM." << endl;
        return "";
    }
    GeminiClient client(apiKey);
    auto result = client.generateCode(prompt);
    if (result.success) return result.code;
    cerr << "Gemini error: " << result.error << endl;
    return "";
}

string Agent::generateDiff(const string& oldCode, const string& newCode) {
    if (oldCode.empty()) return "(first version)";
    if (oldCode == newCode) return "(no change)";
    return ReportGenerator::generateDiff(oldCode, newCode);
}

void Agent::run(const string& problem) {
    cout << "\n=== AGENT START ===" << endl;
    cout << "Problem: " << problem << endl;

    string currentCode;
    string lastError;
    bool success = false;

    for (int iter = 1; iter <= maxIterations; ++iter) {
        cout << "\n--- Iteration " << iter << " ---" << endl;

        // 1. Generate or fix code
        string generatedCode;
        if (iter == 1) {
            GeminiClient client(apiKey);
            auto result = client.generateCode(problem);
            if (result.success) {
                generatedCode = result.code;
            } else {
                cerr << "Failed to generate code: " << result.error << endl;
                break;
            }
        } else {
            GeminiClient client(apiKey);
            auto result = client.fixCode(currentCode, lastError);
            if (result.success) {
                generatedCode = result.code;
            } else {
                cerr << "Failed to fix code: " << result.error << endl;
                break;
            }
        }

        if (generatedCode.empty()) {
            cout << "Failed to generate code." << endl;
            break;
        }

        // 2. Compile
        string compileError;
        bool compiled = compileCode(generatedCode, compileError);

        // 3. Run tests if compiled
        bool testsPassed = false;
        vector<string> failures;
        string testError;
        vector<string> testResultSummaries;

        if (compiled) {
            testsPassed = runTests(EXECUTABLE_PATH, testCasesPath, failures, testError);

            // Build test result summaries
            if (failures.empty()) {
                testResultSummaries.push_back("All tests passed");
            } else {
                for (const auto& f : failures) {
                    testResultSummaries.push_back("FAILED: " + f);
                }
            }
        } else {
            testResultSummaries.push_back("Compilation failed - tests skipped");
        }

        // 4. Record iteration
        string diff = generateDiff(currentCode, generatedCode);
        history.push_back({iter, generatedCode, compiled, compileError, testsPassed,
                           testResultSummaries, failures, diff});

        if (compiled && testsPassed) {
            cout << "Success! All tests passed." << endl;
            success = true;
            break;
        } else {
            if (!compiled) {
                lastError = compileError;
            } else if (!failures.empty()) {
                lastError = "Test failures:\n";
                for (const auto& f : failures) {
                    lastError += "  - " + f + "\n";
                }
            }
            currentCode = generatedCode;
            cout << "Failed. Retrying..." << endl;
        }
    }

    if (!success) {
        cout << "\nMax iterations reached without full success." << endl;
    }
    printFinalReport();
}

void Agent::printFinalReport() {
    vector<string> iterationStatuses;
    vector<vector<string>> allTestResults;

    for (const auto& result : history) {
        string status = result.compiled ? "Compiled" : "Compilation failed";
        if (result.compiled) {
            status += result.testsPassed ? " | Tests passed" : " | Tests failed";
        }
        iterationStatuses.push_back(status);
        allTestResults.push_back(result.testResults);
    }

    string report = ReportGenerator::generateFinalReport(iterationStatuses, allTestResults);
    cout << report << endl;

    // Append diffs for each iteration
    if (!history.empty()) {
        cout << "\n=== DIFF HISTORY ===" << endl;
        for (const auto& result : history) {
            cout << "\n--- Iteration " << result.iteration << " ---" << endl;
            cout << result.diffFromPrevious << endl;
        }
    }

    // Save report to file
    ofstream file("report.md");
    if (file.is_open()) {
        file << "# Autonomous Code Generation Report\n\n";
        file << report;
        file << "\n## Diff History\n\n";
        for (const auto& result : history) {
            file << "### Iteration " << result.iteration << "\n\n";
            file << "```diff\n" << result.diffFromPrevious << "\n```\n\n";
        }
        file.close();
        cout << "\nReport saved to report.md" << endl;
    }
}
