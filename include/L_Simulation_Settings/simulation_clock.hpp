#pragma once
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>
#include <numeric>
#include "thread_name_util.hpp"
#include "logger.hpp"

/**
 * @class SimulationClock
 * @brief 仿真时钟类，用于管理仿真时间推进和同步
 * 
 * 该类实现了线程安全的仿真时钟，支持：
 * 1. 时间推进控制
 * 2. 多线程同步
 * 3. 仿真状态管理
 * 
 * 使用单例模式确保全局只有一个时钟实例
 */
class SimulationClock {
public:
    /**
     * @brief 获取时钟单例实例
     * @return SimulationClock& 时钟实例的引用
     */
    static SimulationClock& getInstance() {
        static SimulationClock instance;
        return instance;
    }

    /**
     * @brief 注册线程
     */
    void registerThread() {
        registered_threads++;
        log_detail("[时钟] 一个线程已注册，总注册线程数: " + std::to_string(registered_threads.load()) + "\n");
    }

    /**
     * @brief 注销线程
     */
    void unregisterThread() {
        registered_threads--;
        log_detail("[时钟] 一个线程已注销，总注册线程数: " + std::to_string(registered_threads.load()) + "\n");
    }

    /**
     * @brief 启动仿真时钟
     * 
     * 重置时钟时间，设置运行状态为true，并通知所有等待的线程
     */
    void start() {
        if (running) return;
        running = true;
        paused = false;
        log_detail("[时钟] 主循环开始，初始化步骤完成标志\n");

        // 先推进一次时间，唤醒所有线程进入第一步
        {
            std::unique_lock<std::mutex> lock(mtx);
            current_time.store(current_time.load() + dt);
            time_steps++;
            log_detail("[时钟] (初始化) 时间步推进: 时间=" + std::to_string(current_time.load()) + ", 步数=" + std::to_string(time_steps.load()) + "\n");
            cv_step_start.notify_all();
        }

        while (running.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(mtx);

            // 1. 等待所有已注册线程完成上一步
            log_detail("[时钟] 等待所有线程完成当前步骤: completed=" + std::to_string(completed_threads.load()) + ", registered=" + std::to_string(registered_threads.load()) + "\n");
            cv_step_end.wait(lock, [this] {
                return completed_threads.load() >= registered_threads.load() || !running.load();
            });

            if (!running.load()) break;

            // 重置完成计数器
            completed_threads = 0;

            // 2. 检查是否暂停，如果暂停则等待恢复
            while (paused.load() && running.load()) {
                log_detail("[时钟] 仿真暂停中，等待恢复...\n");
                cv_step_start.wait(lock, [this] {
                    return !paused.load() || !running.load();
                });
                if (!running.load()) break;
            }

            if (!running.load()) break;

            // 3. 推进时间并通知所有线程开始新步骤
            current_time.store(current_time.load() + dt);
            time_steps++;
            log_detail("[时钟] 时间步推进: 时间=" + std::to_string(current_time.load()) + ", 步数=" + std::to_string(time_steps.load()) + "\n");
            cv_step_start.notify_all();
        }
        log_detail("[时钟] 主循环结束\n");
    }

    /**
     * @brief 停止仿真时钟
     * 
     * 设置运行状态为false，并通知所有等待的线程
     */
    void stop() {
        running = false;
        cv_step_start.notify_all();
        cv_step_end.notify_all();
    }

    /**
     * @brief 暂停仿真时钟
     * 
     * 设置暂停状态为true，时钟将停止推进时间
     */
    void pause() {
        paused = true;
        log_detail("[时钟] 仿真已暂停\n");
    }

    /**
     * @brief 恢复仿真时钟
     * 
     * 设置暂停状态为false，时钟将继续推进时间
     */
    void resume() {
        paused = false;
        log_detail("[时钟] 仿真已恢复\n");
    }

    /**
     * @brief 检查时钟是否暂停
     * @return bool 如果时钟暂停返回true，否则返回false
     */
    bool isPaused() const {
        return paused.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取当前仿真时间（原始double值）
     * @return double 当前仿真时间（秒）
     */
    double getCurrentTime() const {
        return current_time.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取格式化的当前仿真时间字符串
     * @return std::string 格式化的时间字符串，保留两位小数
     */
    std::string getFormattedTime() const {
        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << current_time.load(std::memory_order_acquire);
        return ss.str();
    }

    /**
     * @brief 获取时间步长
     * @return double 时间步长（秒）
     */
    double getTimeStep() const {
        std::lock_guard<std::mutex> lock(mtx);
        return dt;
    }

    /**
     * @brief 获取步骤计数
     * @return int 步骤计数
     */
    int getStepCount() const {
        return time_steps.load(std::memory_order_acquire);
    }

    /**
     * @brief 等待下一个时间步
     */
    void waitForNextStep(size_t last_processed_step) {
        // 日志：进入 waitForNextStep
        log_detail("[时钟] 线程(" + ThreadNaming::get_current_thread_name() +
                          ") 进入 waitForNextStep(), 等待步数 > " + std::to_string(last_processed_step) + "\n");
        std::unique_lock<std::mutex> lock(mtx);
        
        // 等待时钟步数超过上次处理的步数
        cv_step_start.wait(lock, [this, last_processed_step] { 
            return time_steps.load() > last_processed_step || !running; 
        });

        log_detail("[时钟] 线程(" + ThreadNaming::get_current_thread_name() + ") 收到时间步通知，步数=" + std::to_string(time_steps.load()) + "\n");
        // 日志：离开 waitForNextStep
        log_detail("[时钟] 线程(" + ThreadNaming::get_current_thread_name() +
                          ") 离开 waitForNextStep(), 步数=" + std::to_string(time_steps.load()) + "\n");
    }

    /**
     * @brief 通知时钟当前步骤已完成
     */
    void notifyStepCompleted() {
        std::lock_guard<std::mutex> lock(mtx);
        completed_threads++;
        log_detail("[时钟] 线程(" + ThreadNaming::get_current_thread_name() + ") 通知步骤已完成，completed_threads=" + std::to_string(completed_threads.load()) + "/" + std::to_string(registered_threads.load()) + "\n");
        cv_step_end.notify_one();
    }

    /**
     * @brief 检查时钟是否正在运行
     * @return bool 如果时钟正在运行返回true，否则返回false
     */
    bool isRunning() const {
        return running.load(std::memory_order_acquire);
    }

    void setTimeStep(double new_dt) {
        std::lock_guard<std::mutex> lock(mtx);
        dt = new_dt;
    }

private:
    static SimulationClock* instance;
    mutable std::mutex mtx;
    std::condition_variable cv_step_start;
    std::condition_variable cv_step_end;
    std::atomic<uint64_t> time_steps{0};
    double dt;

    std::atomic<bool> running{false};
    std::atomic<double> current_time{0.0};
    std::atomic<int> registered_threads{0};
    std::atomic<int> waiting_threads{0};
    std::atomic<int> completed_threads{0};
    std::atomic<bool> paused{false};

    /**
     * @brief 私有构造函数
     * 
     * 初始化时钟状态：
     * - 当前时间设为0
     * - 上一步时间设为0
     * - 运行状态设为false
     */
    SimulationClock(double step_size = 0.01) : dt(step_size) {}

    /**
     * @brief 默认析构函数
     */
    ~SimulationClock() {
        if (master_thread.joinable()) {
            master_thread.join();
        }
    }

    /**
     * @brief 删除拷贝构造函数
     */
    SimulationClock(const SimulationClock&) = delete;

    /**
     * @brief 删除赋值运算符
     */
    SimulationClock& operator=(const SimulationClock&) = delete;

    std::thread master_thread;

    void masterLoop() {
        while (running.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(mtx);

            // 更新时间和步数
            current_time.store(current_time.load(std::memory_order_acquire) + dt, std::memory_order_release);
            time_steps++;
            log_detail("  [时钟] 时间步更新: 时间=" + std::to_string(current_time.load(std::memory_order_acquire)) + ", 步数=" + std::to_string(time_steps.load(std::memory_order_acquire)) + "\n");

            // 等待一个时间步
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
        }
        log_detail("  [时钟] 主循环结束\n");
    }
}; 