#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <cstdlib>
#include <cstring>
#endif

inline void loadEnvFile(const std::string& filepath = ".env") {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        // Trim whitespace from key
        size_t start = key.find_first_not_of(" \t\r");
        size_t end = key.find_last_not_of(" \t\r");
        if (start != std::string::npos) key = key.substr(start, end - start + 1);

        // Trim whitespace from value
        start = value.find_first_not_of(" \t\r");
        end = value.find_last_not_of(" \t\r");
        if (start != std::string::npos) value = value.substr(start, end - start + 1);

        // Remove surrounding quotes if present
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        if (key.empty()) continue;

#ifdef _WIN32
        std::string envEntry = key + "=" + value;
        putenv(strdup(envEntry.c_str()));
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }
}

#endif // ENV_LOADER_H
