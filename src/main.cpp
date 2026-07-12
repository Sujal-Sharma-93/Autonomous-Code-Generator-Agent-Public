#include "agent/agent.h"
#include "utils/env_loader.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int main() {
    loadEnvFile(".env");

    if (!getenv("GEMINI_API_KEY")) {
        cout << "Warning: GEMINI_API_KEY not set. Agent will not call real API.\n";
        cout << "Set it via environment variable or create a .env file.\n";
    }

    cout << "\n========== Autonomous Code Generator Agent ==========\n";
    cout << "Describe the coding problem:\n> ";
    string problem;
    getline(cin, problem);

    if (problem.empty()) {
        cout << "No problem description provided. Exiting.\n";
        return 1;
    }

    Agent agent(5);
    agent.run(problem);

    return 0;
}
