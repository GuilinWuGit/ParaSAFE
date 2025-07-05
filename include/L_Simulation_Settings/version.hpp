/**
 * @file version.hpp
 * @brief VFT项目版本信息定义
 * 
 * 该文件定义了VFT项目的版本信息，包括版本号、构建信息、版权信息等。
 * 所有需要版本信息的模块都应该包含此头文件。
 */

#ifndef VFT_VERSION_HPP
#define VFT_VERSION_HPP

#include <string>
#include <array>

namespace VFT {

/**
 * @brief 版本信息结构体
 */
struct VersionInfo {
    // 主版本号 - 不兼容的API修改
    static constexpr int MAJOR = 1;
    
    // 次版本号 - 向下兼容的功能性新增
    static constexpr int MINOR = 0;
    
    // 修订号 - 向下兼容的问题修正
    static constexpr int PATCH = 0;
    
    // 预发布标识 (空字符串表示正式版本)
    static constexpr const char* PRERELEASE = "";
    
    // 构建元数据 (构建时间、提交哈希等)
    static constexpr const char* BUILD_METADATA = "";
    
    // 版本字符串
    static std::string getVersionString() {
        std::string version = std::to_string(MAJOR) + "." + 
                             std::to_string(MINOR) + "." + 
                             std::to_string(PATCH);
        
        if (std::string(PRERELEASE).length() > 0) {
            version += "-" + std::string(PRERELEASE);
        }
        
        if (std::string(BUILD_METADATA).length() > 0) {
            version += "+" + std::string(BUILD_METADATA);
        }
        
        return version;
    }
    
    // 完整版本信息
    static std::string getFullVersionInfo() {
        return "VFT " + getVersionString() + 
               " (Build: " + __DATE__ + " " + __TIME__ + ")";
    }
    
    // 版本号数组 (用于比较)
    static constexpr std::array<int, 3> getVersionArray() {
        return {MAJOR, MINOR, PATCH};
    }
    
    // 版本比较
    static bool isNewerThan(int major, int minor, int patch) {
        if (MAJOR > major) return true;
        if (MAJOR < major) return false;
        if (MINOR > minor) return true;
        if (MINOR < minor) return false;
        return PATCH > patch;
    }
    
    // 版本兼容性检查
    static bool isCompatibleWith(int major, int minor) {
        return MAJOR == major && MINOR >= minor;
    }
};

/**
 * @brief 项目信息
 */
struct ProjectInfo {
    // 项目名称
    static constexpr const char* NAME = "Parallel  Situation Awareness & Forcast Environment";
    
    // 项目简称
    static constexpr const char* SHORT_NAME = "ParaSAFE";
    
    // 项目描述
    static constexpr const char* DESCRIPTION = "高保真并行态势感知与预测环境";
    
    // 版权信息
    static constexpr const char* COPYRIGHT = "Copyright (c) 2024 ParaSAFE Team";
    
    // 许可证
    static constexpr const char* LICENSE = "MIT License";
    
    // 项目URL
    static constexpr const char* URL = "https://github.com/your-username/VFT";
    
    // 文档URL
    static constexpr const char* DOCS_URL = "https://your-username.github.io/VFT";
};

/**
 * @brief 构建信息
 */
struct BuildInfo {
    // 构建时间
    static constexpr const char* BUILD_DATE = __DATE__;
    static constexpr const char* BUILD_TIME = __TIME__;
    
    // 编译器信息
    static constexpr const char* COMPILER = 
#ifdef _MSC_VER
        "MSVC";
#elif defined(__GNUC__)
        "GCC";
#elif defined(__clang__)
        "Clang";
#else
        "Unknown";
#endif
    
    // 平台信息
    static constexpr const char* PLATFORM = 
#ifdef _WIN32
        "Windows";
#elif defined(__linux__)
        "Linux";
#elif defined(__APPLE__)
        "macOS";
#else
        "Unknown";
#endif
    
    // 架构信息
    static constexpr const char* ARCHITECTURE = 
#ifdef _WIN64
        "x64";
#elif defined(__x86_64__)
        "x64";
#elif defined(__aarch64__)
        "ARM64";
#else
        "Unknown";
#endif
    
    // 构建类型
    static constexpr const char* BUILD_TYPE = 
#ifdef NDEBUG
        "Release";
#else
        "Debug";
#endif
};

/**
 * @brief 版本信息工具类
 */
class VersionUtils {
public:
    /**
     * @brief 获取版本信息字符串
     */
    static std::string getVersionInfo() {
        return ProjectInfo::NAME + std::string(" ") + VersionInfo::getVersionString();
    }
    
    /**
     * @brief 获取构建信息字符串
     */
    static std::string getBuildInfo() {
        return std::string("Built on ") + BuildInfo::BUILD_DATE + " " + 
               BuildInfo::BUILD_TIME + " with " + BuildInfo::COMPILER + 
               " for " + BuildInfo::PLATFORM + " (" + BuildInfo::ARCHITECTURE + ")";
    }
    
    /**
     * @brief 获取完整信息字符串
     */
    static std::string getFullInfo() {
        return getVersionInfo() + "\n" + getBuildInfo() + "\n" + 
               ProjectInfo::COPYRIGHT + "\n" + ProjectInfo::LICENSE;
    }
    
    /**
     * @brief 检查版本兼容性
     */
    static bool checkCompatibility(int required_major, int required_minor) {
        return VersionInfo::isCompatibleWith(required_major, required_minor);
    }
    
    /**
     * @brief 版本号比较
     */
    static int compareVersions(int major1, int minor1, int patch1,
                              int major2, int minor2, int patch2) {
        if (major1 != major2) return major1 - major2;
        if (minor1 != minor2) return minor1 - minor2;
        return patch1 - patch2;
    }
};

} // namespace VFT

// 全局版本常量 (向后兼容)
#define VFT_VERSION_MAJOR VFT::VersionInfo::MAJOR
#define VFT_VERSION_MINOR VFT::VersionInfo::MINOR
#define VFT_VERSION_PATCH VFT::VersionInfo::PATCH
#define VFT_VERSION_STRING VFT::VersionInfo::getVersionString()
#define VFT_PROJECT_NAME VFT::ProjectInfo::NAME

#endif // VFT_VERSION_HPP 