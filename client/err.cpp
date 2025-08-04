#include "client.hpp"
bool isNotNumber(const std::string& str) {
    // 匹配整数（可选符号 + 数字）
    std::regex integer_regex("^[-+]?\\d+$");
    // 匹配浮点数（可选符号 + 数字 + 可选小数部分）
    std::regex float_regex("^[-+]?\\d*\\.?\\d+$");
    
    return !(
        std::regex_match(str, integer_regex) ||
        std::regex_match(str, float_regex)
    );
}