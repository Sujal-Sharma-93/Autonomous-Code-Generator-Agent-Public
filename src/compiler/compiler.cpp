#include "compiler.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

// Helper function to check if directory exists
static bool directoryExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

// Helper function to create temp directory
static void ensureTempDirectory() {
    if (!directoryExists("temp")) {
#ifdef _WIN32
        system("mkdir temp");
#else
        system("mkdir -p temp");
#endif
    }
}

bool compileCode(const std::string& sourceCode, std::string& errorMessage) {
    // Ensure temp directory exists
    ensureTempDirectory();
    
    // Write source code to temporary file
    std::string sourceFile = "temp/program.c";
#ifdef _WIN32
    std::string executableFile = "temp/program.exe";
#else
    std::string executableFile = "temp/program";
#endif
    
    std::ofstream outFile(sourceFile);
    if (!outFile.is_open()) {
        errorMessage = "Failed to create temporary source file: " + sourceFile;
        return false;
    }
    outFile << sourceCode;
    outFile.close();
    
    // Build GCC command
    std::string compileCommand = "gcc " + sourceFile + " -o " + executableFile + " 2>&1";
    
    // Execute compilation and capture output
    FILE* pipe = popen(compileCommand.c_str(), "r");
    if (!pipe) {
        errorMessage = "Failed to execute GCC compiler";
        return false;
    }
    
    // Read compiler output
    char buffer[128];
    std::string compilerOutput;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        compilerOutput += buffer;
    }
    
    int returnCode = pclose(pipe);
    
    // Check if compilation succeeded (only check return code, not warnings)
    if (returnCode != 0) {
        errorMessage = "Compilation failed:\n" + compilerOutput;
        return false;
    }
    
    // Set executable permission on Unix-like systems
#ifndef _WIN32
    std::string chmodCommand = "chmod +x " + executableFile;
    system(chmodCommand.c_str());
#endif
    
    errorMessage = "";
    return true;
}
