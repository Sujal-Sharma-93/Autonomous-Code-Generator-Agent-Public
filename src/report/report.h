// src/report/report.h
#ifndef REPORT_H
#define REPORT_H

#include <string>
#include <vector>

class ReportGenerator {
public:
    static std::string generateDiff(const std::string& oldCode, const std::string& newCode);
    static std::string generateFinalReport(const std::vector<std::string>& iterationResults, 
                                         const std::vector<std::vector<std::string>>& testResults);
    
private:
    static std::vector<std::string> splitIntoLines(const std::string& code);
};

#endif // REPORT_H

