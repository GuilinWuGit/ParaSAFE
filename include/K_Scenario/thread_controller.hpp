#pragma once
#include <thread>
#include <atomic>
#include <functional>

class ThreadController {
public:
    ThreadController() : running(false) {}
    
    void start(std::function<void()> func) {
        if (running) return;  // 如果已经在运行，直接返回
        running = true;
        thread = std::thread([this, func]() {
            func();  // 只调用一次函数
            running = false;  // 函数执行完后设置运行标志为false
        });
    }
    
    void stop() {
        running = false;
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    bool isRunning() const {
        return running;
    }
    
private:
    std::thread thread;
    std::atomic<bool> running;
}; 