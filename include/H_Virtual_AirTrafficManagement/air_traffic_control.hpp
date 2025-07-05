#pragma once
#include <string>
#include <functional>

/**
 * @brief 虚拟空管模型接口
 */
class IAirTrafficControl {
public:
    virtual ~IAirTrafficControl() = default;
    // 广播指令（如“起飞”“降落”“中止起飞”等）
    virtual void broadcastCommand(const std::string& command) = 0;
    // 注册事件回调（如飞行员收到指令）
    virtual void setOnCommandCallback(std::function<void(const std::string&)> cb) = 0;
};

/**
 * @brief 简单塔台空管实现
 */
class SimpleTowerATC : public IAirTrafficControl {
public:
    void broadcastCommand(const std::string& command) override {
        // 记录/广播指令
        last_command_ = command;
        if (on_command_cb_) on_command_cb_(command);
    }
    void setOnCommandCallback(std::function<void(const std::string&)> cb) override {
        on_command_cb_ = cb;
    }
    std::string getLastCommand() const { return last_command_; }
private:
    std::string last_command_;
    std::function<void(const std::string&)> on_command_cb_;
}; 