/*
 * @file base_controller.hpp
 * @brief 控制器基类头文件
 *
 * 本文件定义了ParaSAFE仿真系统中所有控制器的基类接口。
 * 统一规范了控制器的生命周期管理、状态查询、线程管理等接口，
 * 便于扩展各种类型的控制器（如油门、刹车、巡航、俯仰角等）。
 *
 * 主要功能：
 *   - 统一控制器的启动、停止、状态查询接口
 *   - 提供线程安全的控制器运行机制
 *   - 便于多态扩展和统一管理
 */

#pragma once

// C++头文件
#include <string>      // 字符串类型，用于控制器名称
#include <thread>      // 线程支持，实现控制器独立运行
#include <atomic>      // 原子操作，保证多线程安全

//ParaSAFE头文件
#include "../K_Scenario/shared_state.hpp"   // 共享状态空间，存储仿真系统的所有状态变量
#include "../K_Scenario/event_bus.hpp"      // 事件总线，处理事件发布和订阅

/**
 * @brief 控制器基类
 *
 * 所有具体控制器（如油门、刹车、巡航、俯仰角等）都应继承自本类，
 * 实现其纯虚函数接口，实现各自的控制逻辑。
 *
 * 支持线程安全的启动/停止，便于多线程仿真。
 */
class BaseController {
public:
    /**
     * @brief 虚析构函数，保证派生类资源正确释放
     */
    virtual ~BaseController() = default;

    /**
     * @brief 启动控制器（纯虚函数，需派生类实现）
     * 启动控制器线程，进入控制循环
     */
    virtual void start() = 0;

    /**
     * @brief 停止控制器（纯虚函数，需派生类实现）
     * 停止控制器线程，安全退出
     */
    virtual void stop() = 0;

    /**
     * @brief 查询控制器是否启用（纯虚函数）
     * @return 控制器是否处于启用状态
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief 获取控制器名称（纯虚函数）
     * @return 控制器名称字符串
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 获取当前控制器输出值（纯虚函数）
     * @return 当前控制输出（如油门、刹车、俯仰角等）
     */
    virtual double getCurrentValue() const = 0;

protected:
    SharedStateSpace& state;   ///< 共享状态空间引用，便于控制器读写仿真状态
    EventBus& bus;             ///< 事件总线引用，支持事件驱动控制
    std::atomic<bool> running{false}; ///< 控制器运行状态标志，线程安全
    std::thread controller_thread;    ///< 控制器线程对象，实现独立控制循环

    /**
     * @brief 构造函数
     * @param state 共享状态空间引用
     * @param bus 事件总线引用
     */
    BaseController(SharedStateSpace& state, EventBus& bus) 
        : state(state), bus(bus) {}
}; 