#include "gemini_client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include "../utils/simple_json.h"

GeminiClient::GeminiClient(const std::string& key) : apiKey(key) {}

std::string GeminiClient::stripCodeFences(const std::string& response) {
    std::string code = response;

    size_t start = code.find("```");
    if (start != std::string::npos) {
        size_t newline = code.find('\n', start);
        if (newline != std::string::npos) {
            start = newline + 1;
        } else {
            start = start + 3;
        }

        size_t end = code.find("```", start);
        if (end != std::string::npos) {
            code = code.substr(start, end - start);
        } else {
            code = code.substr(start);
        }
    }

    size_t s = code.find_first_not_of(" \t\n\r");
    size_t e = code.find_last_not_of(" \t\n\r");
    if (s != std::string::npos && e != std::string::npos) {
        code = code.substr(s, e - s + 1);
    }

    return code;
}

std::string GeminiClient::extractGeminiText(const std::string& response) {
    // Strategy 1: Find "parts" array, then "text" within it (most reliable for Gemini API)
    size_t partsPos = response.find("\"parts\"");
    if (partsPos != std::string::npos) {
        size_t textPos = response.find("\"text\"", partsPos);
        if (textPos != std::string::npos) {
            size_t colonPos = response.find(':', textPos + 6);
            if (colonPos != std::string::npos) {
                size_t valStart = colonPos + 1;
                while (valStart < response.size() &&
                       (response[valStart] == ' '  || response[valStart] == '\t' ||
                        response[valStart] == '\n' || response[valStart] == '\r')) {
                    valStart++;
                }

                if (valStart < response.size() && response[valStart] == '"') {
                    size_t strStart = valStart + 1;
                    size_t strEnd = strStart;
                    while (strEnd < response.size()) {
                        if (response[strEnd] == '\\') { strEnd += 2; continue; }
                        if (response[strEnd] == '"') break;
                        strEnd++;
                    }
                    std::string raw = response.substr(strStart, strEnd - strStart);
                    std::cout << "[DEBUG] Extracted raw text length: " << raw.size() << std::endl;
                    return jsonUnescapeString(raw);
                }
            }
        }
    }

    // Strategy 2: Fallback — find any "text" field with a string value
    std::cout << "[DEBUG] parts-based extraction failed, trying fallback..." << std::endl;
    size_t textPos = response.find("\"text\"");
    while (textPos != std::string::npos) {
        size_t colonPos = response.find(':', textPos + 6);
        if (colonPos == std::string::npos) { textPos = response.find("\"text\"", textPos + 6); continue; }

        size_t valStart = colonPos + 1;
        while (valStart < response.size() &&
               (response[valStart] == ' '  || response[valStart] == '\t' ||
                response[valStart] == '\n' || response[valStart] == '\r')) {
            valStart++;
        }

        if (valStart < response.size() && response[valStart] == '"') {
            size_t strStart = valStart + 1;
            size_t strEnd = strStart;
            while (strEnd < response.size()) {
                if (response[strEnd] == '\\') { strEnd += 2; continue; }
                if (response[strEnd] == '"') break;
                strEnd++;
            }
            std::string candidate = response.substr(strStart, strEnd - strStart);
            // Skip candidates that look like JSON objects (safety ratings, etc.)
            if (!candidate.empty() && candidate[0] != '{') {
                std::cout << "[DEBUG] Fallback extracted text length: " << candidate.size() << std::endl;
                return jsonUnescapeString(candidate);
            }
        }

        textPos = response.find("\"text\"", textPos + 6);
    }

    std::cerr << "[DEBUG] All extraction strategies failed" << std::endl;
    return "";
}

bool GeminiClient::verifySource(const std::string& code) {
    if (code.empty()) {
        std::cerr << "[VERIFY] Generated code is empty" << std::endl;
        return false;
    }
    if (code.find("main") == std::string::npos) {
        std::cerr << "[VERIFY] Generated code has no 'main' function" << std::endl;
        return false;
    }
    // Check for broken unicode escapes that weren't decoded
    if (code.find("\\u003c") != std::string::npos ||
        code.find("\\u003e") != std::string::npos ||
        code.find("\\u0026") != std::string::npos) {
        std::cerr << "[VERIFY] Code contains unresolved unicode escapes" << std::endl;
        return false;
    }
    std::cout << "[VERIFY] Source verification passed (" << code.size() << " bytes)" << std::endl;
    return true;
}

std::string GeminiClient::callAPI(const std::string& prompt) {
    if (apiKey.empty()) return "";

    std::string escapedPrompt = jsonEscapeString(prompt);
    std::string jsonBody = "{\"contents\":[{\"parts\":[{\"text\":\"" + escapedPrompt + "\"}]}]}";

    std::string bodyFile = "temp/api_request.json";
#ifdef _WIN32
    system("if not exist temp mkdir temp");
#else
    system("mkdir -p temp");
#endif

    {
        std::ofstream out(bodyFile);
        if (!out.is_open()) return "";
        out << jsonBody;
        out.close();
    }

    std::cout << "\n========== REQUEST JSON ==========\n";
    std::cout << jsonBody << std::endl;

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent?key=" + apiKey;
    std::string outputFile = "temp/api_response.json";

    std::string cmd =
        "curl -s -X POST \"" + url + "\" "
        "-H \"Content-Type: application/json\" "
        "-d @" + bodyFile + " "
        "-o \"" + outputFile + "\" "
        "--max-time 60";

    std::cout << "\n========== CURL COMMAND ==========\n";
    std::cout << cmd << std::endl;

    int ret = system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "curl failed with exit code: " << ret << std::endl;
        return "";
    }

    std::ifstream respFile(outputFile);
    if (!respFile.is_open()) return "";
    std::ostringstream ss;
    ss << respFile.rdbuf();
    std::string response = ss.str();
    respFile.close();

    std::cout << "\n========== RAW API RESPONSE ==========\n";
    std::cout << response << std::endl;

    if (response.empty()) {
        std::cerr << "Empty API response" << std::endl;
        return "";
    }

    std::string errMsg = jsonExtractValue(response, "message");
    if (response.find("\"error\"") != std::string::npos && !errMsg.empty()) {
        std::cerr << "API Error: " << errMsg << std::endl;
        return "";
    }

    return extractGeminiText(response);
}

GenerationResult GeminiClient::generateCode(const std::string& problem) {
    GenerationResult result;
    result.success = false;

    std::string prompt =
        "You are a C programmer. Write a complete, compilable C program for the following problem.\n"
        "Output ONLY the C code. No explanations, no markdown, no code fences.\n"
        "The program must read from stdin and write to stdout.\n\n"
        "Problem: " + problem;

    std::string response = callAPI(prompt);

    if (response.empty()) {
        result.error = "API call failed (check API key and network)";
        return result;
    }

    result.code = stripCodeFences(response);

    if (!verifySource(result.code)) {
        result.error = "Generated code failed verification";
        return result;
    }

    result.success = true;
    return result;
}

GenerationResult GeminiClient::fixCode(const std::string& previousCode, const std::string& errorMessage) {
    GenerationResult result;
    result.success = false;

    std::string prompt =
        "You are a C programmer. The following C code has errors. Fix the code.\n"
        "Output ONLY the corrected C code. No explanations, no markdown, no code fences.\n\n"
        "Previous code:\n" + previousCode + "\n\n"
        "Error:\n" + errorMessage + "\n\n"
        "Fixed code:";

    std::string response = callAPI(prompt);

    if (response.empty()) {
        result.error = "API fix call failed";
        return result;
    }

    result.code = stripCodeFences(response);

    if (!verifySource(result.code)) {
        result.error = "Fixed code failed verification";
        return result;
    }

    result.success = true;
    return result;
}
