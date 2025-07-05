#pragma once
#include <fstream>
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <atomic>
#include "version.hpp"

class Logger {
public:
    enum class Level { BRIEF, DETAIL };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void log(const std::string& msg, Level level) {
        if (!enabled.load()) return;

        std::string timestamp = getTimestamp();
        std::string log_message = timestamp + msg + "\n";

        if (level == Level::BRIEF || level == Level::DETAIL) {
            std::lock_guard<std::mutex> lock(brief_mutex);
            if (brief_file.is_open()) {
                brief_file << log_message;
                brief_file.flush();
            }
        }

        if (level == Level::DETAIL) {
            std::lock_guard<std::mutex> lock(detail_mutex);
            if (detail_file.is_open()) {
                detail_file << log_message;
                detail_file.flush();
            }
        }
    }

    void logVersionInfo() {
        log("=== VFT版本信息 ===", Level::BRIEF);
        log("版本: " + VFT::VersionInfo::getVersionString(), Level::BRIEF);
        log("项目: " + std::string(VFT::ProjectInfo::NAME), Level::BRIEF);
        log("构建时间: " + std::string(VFT::BuildInfo::BUILD_DATE) + " " + 
            std::string(VFT::BuildInfo::BUILD_TIME), Level::BRIEF);
        log("编译器: " + std::string(VFT::BuildInfo::COMPILER), Level::BRIEF);
        log("平台: " + std::string(VFT::BuildInfo::PLATFORM) + " (" + 
            std::string(VFT::BuildInfo::ARCHITECTURE) + ")", Level::BRIEF);
        log("构建类型: " + std::string(VFT::BuildInfo::BUILD_TYPE), Level::BRIEF);
        log("版权: " + std::string(VFT::ProjectInfo::COPYRIGHT), Level::BRIEF);
        log("许可证: " + std::string(VFT::ProjectInfo::LICENSE), Level::BRIEF);
        log("==================", Level::BRIEF);
    }

    void enable() { enabled.store(true); }
    void disable() { enabled.store(false); }
    bool isEnabled() const { return enabled.load(); }

private:
    std::ofstream brief_file;
    std::ofstream detail_file;
    std::mutex brief_mutex;
    std::mutex detail_mutex;
    std::atomic<bool> enabled{true};

    Logger() {
        // 先清空文件
        std::ofstream("output/log_brief.txt", std::ios::out | std::ios::trunc).close();
        std::ofstream("output/log_detail.txt", std::ios::out | std::ios::trunc).close();
        // 再以追加模式打开
        brief_file.open("output/log_brief.txt", std::ios::app);
        detail_file.open("output/log_detail.txt", std::ios::app);
    }
    ~Logger() {
        if (brief_file.is_open()) brief_file.close();
        if (detail_file.is_open()) detail_file.close();
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::tm now_tm;
#ifdef _WIN32
        localtime_s(&now_tm, &now_time_t);
#else
        localtime_r(&now_time_t, &now_tm);
#endif
        std::ostringstream ss;
        ss << std::put_time(&now_tm, "[%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << now_ms.count()
           << "] ";
        return ss.str();
    }
};

inline void log_brief(const std::string& msg) {
    // 写入概要日志文件
    Logger::getInstance().log(msg, Logger::Level::BRIEF);
    // 同时输出到控制台
    std::cout << msg;
    std::cout.flush();
} 

inline void log_detail(const std::string& msg) {
    // 写入详细日志文件
    Logger::getInstance().log(msg, Logger::Level::DETAIL);
} 