#pragma once

#include <string>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <sstream>

namespace ThreadNaming {
    // 使用inline变量，避免多重定义问题 (C++17+)
    inline std::mutex name_map_mutex;
    inline std::unordered_map<std::thread::id, std::string> thread_names;

    /**
     * @brief 为当前线程设置一个名字
     * @param name 要设置的名字
     */
    inline void set_current_thread_name(const std::string& name) {
        std::lock_guard<std::mutex> lock(name_map_mutex);
        thread_names[std::this_thread::get_id()] = name;
    }

    /**
     * @brief 获取当前线程的名字
     * @return std::string 线程的名字。如果未设置，则返回一个基于ID的默认名字。
     */
    inline std::string get_current_thread_name() {
        std::lock_guard<std::mutex> lock(name_map_mutex);
        auto it = thread_names.find(std::this_thread::get_id());
        if (it != thread_names.end()) {
            return it->second;
        }
        
        // 如果名字未注册，返回一个基于ID的默认名字
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return "Thread-" + ss.str();
    }
} 