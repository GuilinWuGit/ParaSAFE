#pragma once
#include <mutex>
#include <map>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>
#include <memory>
#include <atomic>
#include <chrono>
#include <any>
#include <unordered_map>
#include <future>
#include <iomanip>
#include <sstream>
#include "../K_Scenario/shared_state.hpp"
#include "generic_events.hpp"

// 通用事件定义结构 - 事件系统的核心定义
struct EventDefinition {
    std::string name;                    ///< 事件名称
    std::string description;             ///< 事件描述
    std::function<bool(const SharedStateSpace&)> trigger_condition;  ///< 触发条件
    std::vector<GenericEvents::ControllerAction> actions;  ///< 响应动作
    std::string response_description;    ///< 响应动作描述
    bool triggered{false};               ///< 事件触发标志
};

// 事件总线类，用于处理事件订阅和发布
class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;

    // 事件统计结构
    struct EventStats {
        std::atomic<size_t> total_events{0};
        std::atomic<size_t> processed_events{0};
        std::atomic<size_t> dropped_events{0};
        std::atomic<size_t> timeout_events{0};
        std::chrono::steady_clock::time_point last_reset;
    };

    EventBus(SharedStateSpace& state_space) : state(state_space) {
        log_detail("[EventBus] 初始化，事件总线工作线程数: " + std::to_string(MAX_WORKERS) + "\n");
        
        // 创建并启动工作线程
        std::vector<std::promise<void>> thread_ready(MAX_WORKERS);
        std::vector<std::future<void>> thread_futures(MAX_WORKERS);
        
        for (size_t i = 0; i < MAX_WORKERS; ++i) {
            thread_futures[i] = thread_ready[i].get_future();
            worker_threads.emplace_back([this, i, &thread_ready] { 
                log_detail("[EventBus] 事件总线工作线程 " + std::to_string(i) + " 启动\n");
                thread_ready[i].set_value();  // 通知线程已准备就绪
                workerThread(); 
            });
        }
        
        // 等待所有工作线程准备就绪
        for (auto& future : thread_futures) {
            future.wait();
        }
        
        log_detail("[EventBus] 所有工作线程已就绪\n");
    }

    ~EventBus() {
        log_detail("[EventBus] 开始关闭\n");
        {
            std::lock_guard<std::mutex> lock(mtx);
            running = false;
        }
        cv.notify_all();
        for (auto& t : worker_threads) {
            if (t.joinable()) t.join();
        }
        printStats();
        log_detail("[EventBus] 已关闭\n");
    }

    // 订阅事件
    void subscribe(const std::string& event, EventCallback callback) {
        std::lock_guard<std::mutex> lock(mtx);
        subscribers[event].push_back(callback);
        event_stats[event].last_reset = std::chrono::steady_clock::now();
        log_detail("[EventBus] 事件总线初始化订阅事件: " + event + "\n");
    }

    // 发布事件
    void publish(const std::string& event, const std::any& data = std::any{}) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!running) return;

        auto& stats = event_stats[event];
        stats.total_events++;

        if (event_queue.size() >= MAX_QUEUE_SIZE) {
            stats.dropped_events++;
            log_detail("[EventBus] 事件队列已满，丢弃事件: " + event + "\n");
            return;
        }

        event_queue.push({event, data});
        log_detail("[EventBus] 发布事件: " + event + "\n");
        cv.notify_one();
    }

    void printStats() {
        std::lock_guard<std::mutex> lock(mtx);
        log_detail("\n[EventBus] 事件统计:\n");
        for (const auto& [event, stats] : event_stats) {
            log_detail("事件: " + event + "\n");
            log_detail("  总事件数: " + std::to_string(stats.total_events) + "\n");
            log_detail("  已处理: " + std::to_string(stats.processed_events) + "\n");
            log_detail("  已丢弃: " + std::to_string(stats.dropped_events) + "\n");
            log_detail("  超时: " + std::to_string(stats.timeout_events) + "\n");
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        subscribers.clear();
        event_stats.clear();
    }

    bool isEventTriggered(const std::string& event) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx));
        auto it = event_stats.find(event);
        return it != event_stats.end() && it->second.processed_events > 0;
    }

private:
    struct EventItem {
        std::string event;
        std::any data;
    };

    SharedStateSpace& state;
    std::unordered_map<std::string, std::vector<EventCallback>> subscribers;
    std::queue<EventItem> event_queue;
    std::mutex mtx;
    std::vector<std::thread> worker_threads;
    std::condition_variable cv;
    std::atomic<bool> running{true};
    const size_t MAX_WORKERS{4};
    const size_t MAX_QUEUE_SIZE{1000};
    const std::chrono::milliseconds DEFAULT_TIMEOUT{1000};
    std::unordered_map<std::string, EventStats> event_stats;

    void workerThread() {
        while (running) {
            EventItem item;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { 
                    return !event_queue.empty() || !running; 
                });
                
                if (!running) {
                    log_detail("[EventBus] 事件总线工作线程退出\n");
                    return;
                }

                item = event_queue.front();
                event_queue.pop();
            }

            auto it = subscribers.find(item.event);
            if (it != subscribers.end()) {
                auto& stats = event_stats[item.event];
                log_detail("[EventBus] 处理事件: " + item.event + "\n");
                for (const auto& callback : it->second) {
                    try {
                        callback(item.data);
                        stats.processed_events++;
                    } catch (const std::exception& e) {
                        log_detail("[EventBus] 错误：事件处理异常: " + std::string(e.what()) + "\n");
                    } catch (...) {
                        log_detail("[EventBus] 错误：事件处理未知异常\n");
                    }
                }
            } else {
                log_detail("[EventBus] 警告：事件 " + item.event + " 没有订阅者\n");
            }
        }
    }
}; 