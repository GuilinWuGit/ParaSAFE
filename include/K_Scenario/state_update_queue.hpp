#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// 定义状态更新的类型
enum class StateUpdateType {
    Position,
    Velocity,
    Acceleration,
    Throttle,
    Brake
};

// 状态更新消息的结构体
struct StateUpdateMessage {
    StateUpdateType type;
    double value;
};

// 用于状态更新消息的线程安全队列
class StateUpdateQueue {
private:
    std::queue<StateUpdateMessage> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    std::atomic<bool> shutdown_{false};

public:
    // 向队列中推送一条消息
    void push(const StateUpdateMessage& message) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(message);
        }
        cond_var_.notify_one();
    }

    // 尝试从队列中弹出一 条消息（非阻塞）
    bool try_pop(StateUpdateMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        message = queue_.front();
        queue_.pop();
        return true;
    }

    // 通知队列关闭
    void shutdown() {
        shutdown_ = true;
        cond_var_.notify_all();
    }
}; 