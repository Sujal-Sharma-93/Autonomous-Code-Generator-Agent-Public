#ifndef COMPILER_H
#define COMPILER_H

#include <string>

/**
 * Compiles a C source code string using GCC
 * 
 * @param sourceCode The C source code to compile
 * @param errorMessage Output parameter for error messages if compilation fails
 * @return true if compilation succeeds, false otherwise
 */
bool compileCode(const std::string& sourceCode, std::string& errorMessage);

#endif // COMPILER_H
