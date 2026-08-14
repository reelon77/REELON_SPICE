#include "tokenize.h"
#include <cctype>
#include <algorithm>

// 外层由getline进行读取
std::vector<std::string> tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    // 直接通过 空格、制表符、回车 分割
    size_t start = 0;
    size_t end = 0;
    // 跳过开头的空白字符
    while (start < input.length() && (input[start] == ' ' || input[start] == '\t' || input[start] == '\n' || input[start] == '\r')) {
        start++;
    }
    end = start;
    if (start < input.length() && input[start] == '*') {
        // 注释行，直接返回空向量
        return tokens;
    }
    while (end < input.length()) {
        if (input[end] == ' ' || input[end] == '\t' || input[end] == '\n' || input[end] == '\r') {
            if (start < end) {
                tokens.push_back(input.substr(start, end - start));
            }
            start = end + 1;
            while (start < input.length() && (input[start] == ' ' || input[start] == '\t' || input[start] == '\n' || input[start] == '\r')) {
                start++;
            }
            end = start;
        } else {
            end++;
        }
    }
    if (start < input.length()) {
        tokens.push_back(input.substr(start));
    }
    for (auto& token : tokens) {
        std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }
    return tokens;
}
