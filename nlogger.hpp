#pragma once

#include <string>
#include <vector>
#include <initializer_list>
#include "esp_log.h"

class NLogger 
{
private:
    std::string prefix_;
    std::string separator_;
    std::string suffix_;
    std::string tags_;
    std::vector<std::string> tags_list_;

    void RebuildTags();

public:
    NLogger();
    NLogger(const std::string& prefix); // Add this constructor
    ~NLogger();
    
    void SetPrefix(const std::string& prefix);
    void SetSeparator(const std::string& separator);
    void SetSuffix(const std::string& suffix);
    void SetTags(const std::string& tags);
    void SetTags(const std::vector<std::string>& tags);
    void SetTags(const std::initializer_list<std::string>& tags);
    void SetTag(int index, const std::string& tag);
    void ClearTags();

    template<typename... Args>
    void Verbose(const char* fmt, Args... args) {
        Log(ESP_LOG_VERBOSE, fmt, args...);
    }
    template<typename... Args>
    void Debug(const char* fmt, Args... args) {
        Log(ESP_LOG_DEBUG, fmt, args...);
    }
    template<typename... Args>
    void Info(const char* fmt, Args... args) {
        Log(ESP_LOG_INFO, fmt, args...);
    }
    template<typename... Args>
    void Warning(const char* fmt, Args... args) {
        Log(ESP_LOG_WARN, fmt, args...);
    }
    template<typename... Args>
    void Error(const char* fmt, Args... args) {
        Log(ESP_LOG_ERROR, fmt, args...);
    }
    template<typename... Args>
    void Fatal(const char* fmt, Args... args) {
        Log(ESP_LOG_ERROR, fmt, args...);
    }

private:
    template<typename... Args>
    void Log(esp_log_level_t level, const char* fmt, Args... args) {
        std::string full_fmt = prefix_ + tags_;
        if (!separator_.empty()) {
            full_fmt += separator_;
        }
        full_fmt += fmt;
        full_fmt += suffix_;
        
        esp_log_write(level, "NLogger", full_fmt.c_str(), args...);
    }
};
