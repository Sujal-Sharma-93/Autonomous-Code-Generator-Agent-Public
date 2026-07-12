#include "tester.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "../utils/simple_json.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

#ifdef _WIN32
struct RunContext {
    std::string command;
    std::string actualOutput;
    bool finished;
};

static unsigned __stdcall runWithTimeout(void* param) {
    RunContext* ctx = static_cast<RunContext*>(param);
    FILE* pipe = popen(ctx->command.c_str(), "r");
    if (pipe) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            ctx->actualOutput += buffer;
        }
        pclose(pipe);
    }
    ctx->finished = true;
    return 0;
}

static bool runExecutable(const std::string& command, std::string& output, int timeoutSeconds) {
    RunContext ctx;
    ctx.command = command;
    ctx.finished = false;

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, runWithTimeout, &ctx, 0, NULL);
    if (!hThread) return false;

    DWORD waitResult = WaitForSingleObject(hThread, timeoutSeconds * 1000);
    if (waitResult == WAIT_TIMEOUT) {
        TerminateThread(hThread, 1);
        CloseHandle(hThread);
        return false; // timed out
    }
    CloseHandle(hThread);
    output = ctx.actualOutput;
    return true;
}
#else
static bool runExecutable(const std::string& command, std::string& output, int timeoutSeconds) {
    int pipefd[2];
    if (pipe(pipefd) != 0) return false;

    pid_t pid = fork();
    if (pid == 0) {
        // Child: redirect stdout to pipe, run command via shell
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        _exit(127);
    } else if (pid > 0) {
        close(pipefd[1]);
        // Parent: read with timeout using alarm + read
        struct timeval start, now;
        gettimeofday(&start, NULL);

        // Set non-blocking or use alarm
        char buffer[256];
        bool timedOut = false;
        fd_set fds;
        struct timeval tv;

        while (true) {
            FD_ZERO(&fds);
            FD_SET(pipefd[0], &fds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            int sel = select(pipefd[0] + 1, &fds, NULL, NULL, &tv);
            if (sel > 0) {
                ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    output += buffer;
                } else if (n == 0) {
                    break; // EOF
                }
            } else if (sel == 0) {
                // Timeout on select, check total elapsed
                gettimeofday(&now, NULL);
                double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1000000.0;
                if (elapsed >= timeoutSeconds) {
                    timedOut = true;
                    break;
                }
            } else {
                break; // error
            }
        }

        close(pipefd[0]);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return !timedOut;
    } else {
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }
}
#endif

bool runTests(const std::string& executablePath,
              const std::string& testCasesPath,
              std::vector<std::string>& failedTests,
              std::string& errorMessage,
              int timeoutSeconds) {

    failedTests.clear();

    // Check if executable exists
    std::ifstream execFile(executablePath);
    if (!execFile.good()) {
        errorMessage = "Executable not found: " + executablePath;
        return false;
    }
    execFile.close();

    // Load test cases
    std::vector<JsonTestCase> testCases = parseTestCases(testCasesPath);
    if (testCases.empty()) {
        errorMessage = "No test cases found in: " + testCasesPath;
        return false;
    }

    // Ensure temp directory exists
#ifdef _WIN32
    system("if not exist temp mkdir temp");
#else
    system("mkdir -p temp");
#endif

    bool allPassed = true;

    for (const auto& tc : testCases) {
        // Write input to temp file (safe, no shell injection)
        std::string inputPath = "temp/test_input.txt";
        {
            std::ofstream inFile(inputPath);
            if (!inFile.is_open()) {
                failedTests.push_back(tc.name);
                allPassed = false;
                continue;
            }
            inFile << tc.input << "\n";
            inFile.close();
        }

        // Run executable with input redirected from file
        std::string command = "\"" + executablePath + "\" < " + inputPath + " 2>&1";

        std::string actualOutput;
        bool ok = runExecutable(command, actualOutput, timeoutSeconds);

        if (!ok) {
            failedTests.push_back(tc.name + " (TIMEOUT)");
            allPassed = false;
            std::cerr << "Test timed out: " << tc.name << std::endl;
            continue;
        }

        // Compare output
        if (trim(actualOutput) != trim(tc.expected_output)) {
            failedTests.push_back(tc.name);
            allPassed = false;

            std::cerr << "Test failed: " << tc.name << std::endl;
            std::cerr << "  Expected: '" << trim(tc.expected_output) << "'" << std::endl;
            std::cerr << "  Got:      '" << trim(actualOutput) << "'" << std::endl;
        }
    }

    errorMessage = failedTests.empty() ? "" : std::to_string(failedTests.size()) + " test(s) failed";
    return allPassed;
}
