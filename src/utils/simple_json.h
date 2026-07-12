#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdio>

struct JsonTestCase {
    std::string name;
    std::string input;
    std::string expected_output;
};

inline std::string jsonEscapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + s.size() / 4);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned int)c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
                break;
        }
    }
    return out;
}

inline std::string jsonUnescapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '"':  out += '"';  i++; break;
                case '\\': out += '\\'; i++; break;
                case 'n':  out += '\n'; i++; break;
                case 'r':  out += '\r'; i++; break;
                case 't':  out += '\t'; i++; break;
                case 'b':  out += '\b'; i++; break;
                case 'f':  out += '\f'; i++; break;
                case 'u':
                {
                    if (i + 5 < s.size()) {
                        std::string hex = s.substr(i + 2, 4);
                        unsigned int cp = 0;
                        bool valid = true;
                        for (char h : hex) {
                            cp <<= 4;
                            if (h >= '0' && h <= '9')      cp |= (unsigned int)(h - '0');
                            else if (h >= 'a' && h <= 'f')  cp |= (unsigned int)(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F')  cp |= (unsigned int)(h - 'A' + 10);
                            else { valid = false; break; }
                        }
                        if (valid) {
                            if (cp < 0x80) {
                                out += static_cast<char>(cp);
                            } else if (cp < 0x800) {
                                out += static_cast<char>(0xC0 | (cp >> 6));
                                out += static_cast<char>(0x80 | (cp & 0x3F));
                            } else {
                                out += static_cast<char>(0xE0 | (cp >> 12));
                                out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                                out += static_cast<char>(0x80 | (cp & 0x3F));
                            }
                        } else {
                            out += '?';
                        }
                        i += 5;
                    }
                    break;
                }
                default:
                    out += s[i];
                    break;
            }
        } else {
            out += s[i];
        }
    }

    return out;
}

inline std::string jsonExtractValue(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";
    pos++;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) pos++;

    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        pos++;
        size_t end = pos;
        while (end < json.size()) {
            if (json[end] == '\\') { end += 2; continue; }
            if (json[end] == '"') break;
            end++;
        }
        return jsonUnescapeString(json.substr(pos, end - pos));
    } else {
        size_t end = pos;
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n') end++;
        return json.substr(pos, end - pos);
    }
}

inline std::vector<JsonTestCase> parseTestCases(const std::string& filepath) {
    std::vector<JsonTestCase> cases;

    std::ifstream file(filepath);
    if (!file.is_open()) return cases;

    std::string content;
    std::string line;
    while (std::getline(file, line)) content += line;
    file.close();

    size_t pos = 0;
    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

        size_t objEnd = content.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = content.substr(objStart, objEnd - objStart + 1);

        JsonTestCase tc;
        tc.name = jsonExtractValue(obj, "name");
        tc.input = jsonExtractValue(obj, "input");
        tc.expected_output = jsonExtractValue(obj, "expected_output");

        if (tc.name.empty()) tc.name = "Unnamed test";
        cases.push_back(tc);

        pos = objEnd + 1;
    }

    return cases;
}

#endif // SIMPLE_JSON_H
