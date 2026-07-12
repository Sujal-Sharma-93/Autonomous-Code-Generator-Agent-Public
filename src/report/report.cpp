// src/report/report.cpp
#include "report.h"
#include <sstream>
#include <algorithm>

std::vector<std::string> ReportGenerator::splitIntoLines(const std::string& code) {
    std::vector<std::string> lines;
    std::istringstream stream(code);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

std::string ReportGenerator::generateDiff(const std::string& oldCode, const std::string& newCode) {
    auto oldLines = splitIntoLines(oldCode);
    auto newLines = splitIntoLines(newCode);
    
    std::ostringstream diffOutput;
    size_t i = 0, j = 0;
    
    while (i < oldLines.size() && j < newLines.size()) {
        if (oldLines[i] == newLines[j]) {
            diffOutput << "  " << oldLines[i] << "\n";
            i++;
            j++;
        } else {
            // Look ahead to see if this is a deletion followed by insertion
            bool foundMatch = false;
            for (size_t k = 1; k <= 3 && j + k < newLines.size(); k++) {
                if (oldLines[i] == newLines[j + k]) {
                    for (size_t l = 0; l < k; l++) {
                        diffOutput << "+ " << newLines[j + l] << "\n";
                    }
                    j += k;
                    foundMatch = true;
                    break;
                }
            }
            
            if (!foundMatch) {
                diffOutput << "- " << oldLines[i] << "\n";
                if (j < newLines.size()) {
                    diffOutput << "+ " << newLines[j] << "\n";
                    j++;
                }
                i++;
            }
        }
    }
    
    // Add remaining lines
    while (i < oldLines.size()) {
        diffOutput << "- " << oldLines[i] << "\n";
        i++;
    }
    while (j < newLines.size()) {
        diffOutput << "+ " << newLines[j] << "\n";
        j++;
    }
    
    return diffOutput.str();
}
// Continue in src/report/report.cpp
std::string ReportGenerator::generateFinalReport(
    const std::vector<std::string>& iterationResults,
    const std::vector<std::vector<std::string>>& testResults) {
    
    std::ostringstream report;
    
    report << "=== AUTONOMOUS CODE GENERATION REPORT ===\n\n";
    report << "Total iterations: " << iterationResults.size() << "\n\n";
    
    for (size_t i = 0; i < iterationResults.size(); i++) {
        report << "--- Iteration " << (i + 1) << " ---\n";
        report << "Status: " << iterationResults[i] << "\n";
        
        if (i < testResults.size()) {
            report << "Test Results:\n";
            for (const auto& test : testResults[i]) {
                report << "  - " << test << "\n";
            }
        }
        report << "\n";
    }
    
    return report.str();
}
