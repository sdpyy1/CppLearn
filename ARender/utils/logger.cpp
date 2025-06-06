#pragma once

#include <qdebug.h>

#include "logger.h"

Logger::LogLevel Logger::_level = Logger::LogLevel::LOCALLOGLEVEL;
std::string Logger::_previous = "";

void Logger::setLogLevel(LogLevel level) {
    _level = level;
}

void Logger::log(LogLevel level, std::string message) {
    if (level < _level) return;
    if (message == _previous) return;
    _previous = message;

    switch (level) {
    case DEBUG:
        qDebug() << "DEBUG: " << message.c_str();
        break;
    case INFO:
        qDebug() << "INFO: " << message.c_str();
        break;
    case WARNING:
        qDebug() << "WARNING: " << message.c_str();
        break;
    case ERR:
        qDebug() << "ERROR: " << message.c_str();
        break;
    }
}

void Logger::debug(std::string message) {
    log(DEBUG, message);
}

void Logger::info(std::string message) {
    log(INFO, message);
}

void Logger::warning(std::string message) {
    log(WARNING, message);
}

void Logger::error(std::string message) {
    log(ERR, message);
}
