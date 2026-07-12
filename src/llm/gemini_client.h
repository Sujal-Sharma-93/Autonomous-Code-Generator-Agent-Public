#ifndef GEMINI_CLIENT_H
#define GEMINI_CLIENT_H

#include <string>

struct GenerationResult {
    bool success;
    std::string code;
    std::string error;
};

class GeminiClient {
public:
    GeminiClient(const std::string& apiKey);
    GenerationResult generateCode(const std::string& problem);
    GenerationResult fixCode(const std::string& previousCode, const std::string& errorMessage);

private:
    std::string apiKey;
    std::string callAPI(const std::string& prompt);
    std::string extractGeminiText(const std::string& response);
    bool verifySource(const std::string& code);
    static std::string stripCodeFences(const std::string& response);
};

#endif
