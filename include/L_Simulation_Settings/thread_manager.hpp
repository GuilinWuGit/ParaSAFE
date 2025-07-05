#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <chrono>
#include <windows.h>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>

class ThreadManager {
private:
    std::atomic<bool> emergency_stop{false};
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> managed_threads;
    std::chrono::steady_clock::time_point start_time;

    // 性能统计
    struct ThreadStats {
        std::string name;
        long long duration_ms;
        std::chrono::steady_clock::time_point last_active;
    };
    mutable std::unordered_map<std::thread::id, ThreadStats> stats;
    mutable std::mutex stats_mutex;

public:
    ThreadManager() : emergency_stop(false) {
        start_time = std::chrono::steady_clock::now();
        log_detail("[ThreadManager] 初始化\n");
    }

    ~ThreadManager() {
        emergency_stop = true;
        cv.notify_all();
        for (auto& t : managed_threads) {
            if (t.joinable()) t.join();
        }
        printStats();
        log_detail("[ThreadManager] 已关闭\n");
    }

    // 启动受管理的线程
    template<typename Func>
    void startThread(Func&& func, const std::string& name, int priority = THREAD_PRIORITY_NORMAL) {
        std::thread t([this, func = std::forward<Func>(func), name, priority]() {
            try {
                // 设置线程优先级
                SetThreadPriority(GetCurrentThread(), priority);
                
                // 记录线程开始时间
                auto start_time = std::chrono::steady_clock::now();
                
                // 执行线程函数
                func();
                
                // 更新统计信息
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                
                {
                    std::lock_guard<std::mutex> lock(stats_mutex);
                    stats[std::this_thread::get_id()] = {
                        name,
                        duration.count(),
                        std::chrono::steady_clock::now()
                    };
                }
            } catch (const std::exception& e) {
                log_detail("[ThreadManager] 线程异常: " + std::string(e.what()) + "\n");
                emergency_stop = true;
            } catch (...) {
                log_detail("[ThreadManager] 线程发生未知异常\n");
                emergency_stop = true;
            }
        });
        
        t.detach();
    }

    // 等待所有线程就绪
    void waitForReady() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return emergency_stop.load(); });
    }

    // 检查是否处于紧急停止状态
    bool isEmergencyStop() const {
        return emergency_stop;
    }

    // 触发紧急停止
    void triggerEmergencyStop() {
        emergency_stop = true;
        cv.notify_all();
    }

    void printStats() const {
        std::lock_guard<std::mutex> lock(stats_mutex);
        log_detail("\n线程统计信息:\n");
        for (const auto& [id, stat] : stats) {
            std::stringstream ss;
            ss << "线程 ID: " << std::hash<std::thread::id>{}(id) << "\n"
               << "名称: " << stat.name << "\n"
               << "运行时间: " << stat.duration_ms << "ms\n"
               << "最后活动: " << std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::steady_clock::now() - stat.last_active).count() << "s前\n";
            log_detail(ss.str());
        }
    }
}; 