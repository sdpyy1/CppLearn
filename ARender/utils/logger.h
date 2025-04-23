#pragma once
#include<sstream>
#include <string>
#include <cstdio>
#include <memory>
#include <vector>
#include<iostream>
#define LOCALLOGLEVEL DEBUG

class Logger {
public:
    enum LogLevel { DEBUG, INFO, WARNING, ERR };

private:
    static LogLevel _level;
    static std::string _previous;

    template<typename T>
    static std::string toString(T&& arg) {
        if constexpr (std::is_same_v<std::decay_t<T>, char*>) {
            return arg ? arg : "(null)";
        }
        else if constexpr (std::is_same_v<std::decay_t<T>, const char*>) {
            return arg ? arg : "(null)";
        }
        else if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
            return std::to_string(arg);
        }
        else {
            // 对于其他类型，尝试使用流操作
            std::ostringstream oss;
            oss << arg;
            return oss.str();
        }
    }

    template<typename... Args>
    static std::string formatString(const std::string& format, Args... args) {
        std::string result;
        size_t last_pos = 0;
        size_t pos = format.find("{}");

        auto handleArg = [&](auto&& arg) {
            pos = format.find("{}", last_pos);
            if(pos != std::string::npos) {
                result += format.substr(last_pos, pos - last_pos);
                result += toString(arg);
                last_pos = pos + 2;
            }
        };

        (handleArg(args), ...);
        result += format.substr(last_pos);
        return result;
    }

public:
    static void setLogLevel(LogLevel level);
    static void log(LogLevel level, std::string message);

    // 旧版接口 - 保持兼容
    static void debug(std::string message);
    static void info(std::string message);
    static void warning(std::string message);
    static void error(std::string message);

    // 新版格式化接口 - 使用 {} 占位符
    template<typename... Args>
    static void debug(const std::string& format, Args... args) {
        log(DEBUG, formatString(format, args...));
    }

    template<typename... Args>
    static void info(const std::string& format, Args... args) {
        log(INFO, formatString(format, args...));
    }

    template<typename... Args>
    static void warning(const std::string& format, Args... args) {
        log(WARNING, formatString(format, args...));
    }

    template<typename... Args>
    static void error(const std::string& format, Args... args) {
        log(ERR, formatString(format, args...));
    }
};
