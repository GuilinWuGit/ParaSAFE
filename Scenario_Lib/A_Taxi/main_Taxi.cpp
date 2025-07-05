/********************************************************************************************************************
 * @file main_Taxi.cpp
 * @brief ParaSAFE推荐场景主程序模板，适用于所有飞行仿真场景
 * 
 * 该文件定义了ParaSAFE仿真的主程序,这也是ParaSAFE框架下唯一一个cpp程序，其他的功能都用头文件的方式实现，大大降低了大家的学习和使用门槛。
 * 
 * 强烈推荐使用该模版构建新的飞行场景：你只需要定义的目标场景相关的代码，其他保持不变，出错概率将降到极低。鼓励大家共享场景到Github。
 * 
 * ******************************************************************************************************************/ 

// 系统头文件: 所有飞行场景都加上这些头文件，这些头文件支持了ParaSAFE的代码编写
#include <iostream>           //C++系统头文件,  标准输入输出流
#include <windows.h>          //C++系统头文件,  Windows API，控制台编码设置等
#include <thread>             //C++系统头文件,  线程库，支持多线程
#include <chrono>             //C++系统头文件,  时间库，计时与延迟
#include <atomic>             //C++系统头文件,  原子操作库，线程安全变量
#include <memory>             //C++系统头文件,  智能指针库
#include <sstream>            //C++系统头文件,  字符串流库，格式化输出
#include <iomanip>            //C++系统头文件,  输入输出流格式控制库
#include <mutex>              //C++系统头文件,  互斥锁库，线程同步
#include <vector>             //C++系统头文件,  向量容器
#include <cmath>              //C++系统头文件,  数学库，数学函数
#include <fstream>            //C++系统头文件,  文件流库，文件读写
#include <string>             //C++系统头文件,  字符串库
#include <cstdlib>            //C++系统头文件,  标准库，包含通用工具函数
#include <condition_variable> //C++系统头文件,  条件变量，线程同步
#include <functional>         //C++系统头文件,  函数对象和回调
#include <queue>              //C++系统头文件,  队列容器
#include <any>                //C++系统头文件,  任意类型容器
#include <unordered_map>      //C++系统头文件,  哈希表容器

// ParaSAFE头文件库: 所有飞行场景都加上这些头文件，这是ParaSAFE系统为大家提供的标准化头文件，也是ParaSAFE系统最核心的文件，它们一起构建了ParaSAFE系统的核心框架
#include "../../include/K_Scenario/shared_state.hpp"                      // ParaSAFE系统头文件, 共享状态空间，仿真全局状态
#include "../../include/K_Scenario/event_bus.hpp"                         // ParaSAFE系统头文件, 事件总线，事件发布与订阅
#include "../../include/K_Scenario/controller_manager.hpp"                // ParaSAFE系统头文件, 控制器管理器，管理各类控制器
#include "../../include/D_DynamicModel/DynamicsModel_FixedWing_Linear.hpp"                // ParaSAFE系统头文件, 固定翼线性动力学模型，推进物理状态
#include "../../include/L_Simulation_Settings/simulation_clock.hpp"       // ParaSAFE系统头文件, 仿真时钟，统一时间推进
#include "../../include/L_Simulation_Settings/thread_manager.hpp"         // ParaSAFE系统头文件, 线程管理器，线程统一管理
#include "../../include/L_Simulation_Settings/simulation_manager.hpp"     // ParaSAFE系统头文件, 仿真管理器，仿真流程控制
#include "../../include/L_Simulation_Settings/data_recorder.hpp"          // ParaSAFE系统头文件, 文件日志器，数据记录与输出
#include "../../include/L_Simulation_Settings/simulation_config_base.hpp" // ParaSAFE系统头文件, 仿真配置基类，参数管理
#include "../../include/C_Flight_Control/controller_config.hpp"                 // ParaSAFE系统头文件, 控制器参数配置
#include "../../include/K_Scenario/controller_actions_config.hpp"         // ParaSAFE系统头文件, 控制器动作配置，事件-动作映射
#include "../../include/K_Scenario/state_update_queue.hpp"                // ParaSAFE系统头文件, 状态更新队列，线程间状态同步
#include "../../include/L_Simulation_Settings/state_manager_thread.hpp"   // ParaSAFE系统头文件, 状态空间线程，管理状态推进
#include "../../include/L_Simulation_Settings/logger.hpp"                 // ParaSAFE系统头文件, 日志模块，详细/简要日志输出
#include "../../include/K_Scenario/event_detection.hpp"                   // ParaSAFE系统头文件, 通用事件检测线程头文件

// 本科目（中断起飞）头文件：每个科目都需要这几个头文件，如果你在新建一个场景，则需要重新定义这几个头文件
#include "Taxi_config.hpp"          // 本科目头文件, 配置文件，参数定义
#include "Taxi_events.hpp"          // 本科目头文件, 事件定义，事件枚举与条件
#include "Taxi_initial_state.hpp"   // 本科目头文件, 初始状态，仿真初始值

// 本科目（中断起飞）头文件_应用的控制器，ParaSAFE提供的控制器示例，如果你是初学者，你可以使用这些控制器来支持你的场景构建；如果你是资深开发者，你可以自行开发控制器，欢迎共享到Github社区
#include "../../include/C_Flight_Control/throttle_controller.hpp" // 本科目头文件_应用的控制器, 油门控制器
#include "../../include/C_Flight_Control/brake_controller.hpp"    // 本科目头文件_应用的控制器, 刹车控制器
#include "../../include/C_Flight_Control/CruiseOnRunway_controller.hpp"   // 本科目头文件_应用的控制器, 跑道巡航控制器

#include "A_Aircraft_Configuration/aircraft_config.hpp"
#include "A_Aircraft_Configuration/AircraftConfig_FixedWin_AC1.hpp"
#include "A_Aircraft_Configuration/AircraftConfig_FixedWin_AC2.hpp"

int main() {

    // =============================== 机型选择 =============================== //
    // 选择机型（可通过配置、命令行等方式扩展）
    std::shared_ptr<AircraftConfigBase> aircraftConfig = std::make_shared<AircraftConfig_FixedWin_AC1>();
    // 若需切换为AC2机型，只需如下：
    // std::shared_ptr<AircraftConfigBase> aircraftConfig = std::make_shared<AircraftConfig_FixedWin_AC2>();

    // =============================== 力学模型选择============================= //
    // 选择力学模型（可通过配置、命令行等方式扩展）
    std::shared_ptr<IForceModel> forceModel = std::make_shared<ACForceModel>();
    // 若需切换为非线性模型，只需如下：
    // std::shared_ptr<IForceModel> forceModel = std::make_shared<ACForceModel_Nonlinear>();

    // =============================== 动力学模型选择 =============================== //
    // 选择动力学模型（可通过配置、命令行等方式扩展）
    std::shared_ptr<IDynamicsModel> dynamicsModel = std::make_shared<DynamicsModel_FixedWing_Linear>();
    // 若需切换为非线性模型，只需如下：
    // std::shared_ptr<IDynamicsModel> dynamicsModel = std::make_shared<DynamicsModel_FixedWing_Nonlinear>();

    // =============================== 初始化   ============================== // 
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "[主函数] 开始加载配置文件..." << std::endl;
    TaxiConfig::loadConfig("Taxi_config.txt");
    ControllerActionsConfig::loadConfig("controller_actions_config.txt");
    std::cout << "[主函数] 配置文件加载完成" << std::endl;
    log_brief("=========   仿真开始  ========= \n");
    SharedStateSpace state;
    TaxiInitialState::initializeMotionState(state, aircraftConfig);
    state.simulation_started = true;
    state.setSimulationRunning(true);
    log_brief("[主函数：状态空间] 状态空间已初始化\n");
    EventBus bus(state);
    log_brief("[主函数：事件总线] 事件总线已初始化\n");
    StateUpdateQueue update_queue;
    log_brief("[主函数：队列] 状态更新队列已初始化\n");
    ControllerManagerThread controller_manager_thread(state, bus, update_queue);
    log_brief("[主函数：控制器管理器] 控制器管理器已初始化\n");
    controller_manager_thread.setEventDefinitions(TaxiEvents::EVENT_DEFINITIONS);
    EventMonitorThread event_monitor_thread(state, bus, TaxiEvents::EVENT_DEFINITIONS);
    log_brief("[主函数：事件监控] 事件监控器已初始化\n");
    SimulationControlThread simulation_control_thread(state, bus);
    log_brief("[主函数：仿真控制] 仿真控制线程已初始化\n");
    controller_manager_thread.setupEventHandlers();
    log_brief("[主函数：事件处理] 事件处理器已设置\n");
    FileLogger logger("Taxi_log.txt");
    DataRecorderThread data_recorder_thread(state, SimulationClock::getInstance(), logger);
    auto start_data_recorder = [&]() {
        data_recorder_thread.start();
        log_brief("[主函数：数据输出] 数据输出线程已启动\n");
    };
    auto stop_data_recorder = [&]() {
        data_recorder_thread.stop();
        data_recorder_thread.join();
        log_brief("[主函数：数据输出] 数据输出线程已停止\n");
    };
    log_brief("=== 初始化结束，开始启动各线程 ===\n");
    std::thread clock_thread;
    bool clock_thread_started = false;
    auto start_clock = [&]() {
        if (!clock_thread_started) {
            clock_thread = std::thread([]() {
                ThreadNaming::set_current_thread_name("SimulationClock");
                SimulationClock::getInstance().start();
            });
            clock_thread_started = true;
            log_brief("[主函数：时钟] 仿真时钟线程已启动\n");
        }
    };
    auto stop_clock = [&]() {
        SimulationClock::getInstance().stop();
        log_brief("[主函数：时钟] 仿真时钟已停止\n");
        if (clock_thread_started && clock_thread.joinable()) {
            clock_thread.join();
            log_brief("[主函数：时钟] 仿真时钟线程已停止\n");
        }
    };
    auto start_event_monitor = [&]() {
        event_monitor_thread.start();
        log_brief("[主函数：事件监控] 事件监控线程已启动\n");
    };
    auto stop_event_monitor = [&]() {
        event_monitor_thread.stop();
        event_monitor_thread.join();
        log_brief("[主函数：事件监控] 事件监控线程已停止\n");
    };
    auto start_controller_manager = [&]() {
        controller_manager_thread.start();
        log_brief("[主函数：控制器管理器] 控制器管理线程已启动\n");
    };
    auto stop_controller_manager = [&]() {
        controller_manager_thread.stop();
        controller_manager_thread.join();
        log_brief("[主函数：控制器管理器] 控制器管理线程已停止\n");
    };
    StateManagerThread state_manager(state, update_queue, SimulationClock::getInstance());
    auto start_state_manager = [&]() {
        state_manager.start();
        log_brief("[主函数：状态空间] 状态空间线程已启动\n");
    };
    auto stop_state_manager = [&]() {
        state_manager.stop();
        log_brief("[主函数：状态空间线程] 状态空间线程已停止\n");
    };

    std::thread dynamics_thread;
    bool dynamics_thread_started = false;
    auto start_dynamics = [&]() {
        if (!dynamics_thread_started) {
            dynamics_thread = std::thread([&state, &update_queue, &bus, aircraftConfig, forceModel, dynamicsModel]() {
                ThreadNaming::set_current_thread_name("DynamicsModel");
                log_brief("[主函数：动力学模型] 动力学模型线程已启动\n");
                SimulationClock::getInstance().registerThread();
                size_t current_step = 0;
                while (state.simulation_running.load(std::memory_order_acquire)) {
                    log_brief("[主函数：动力学模型] 等待下一个时间步\n");
                    SimulationClock::getInstance().waitForNextStep(current_step);
                    current_step = SimulationClock::getInstance().getStepCount();
                    if (!SimulationClock::getInstance().isRunning()) {
                        log_brief("[主函数：动力学模型] 时钟已停止，退出循环\n");
                        break;
                    }
                    state.simulation_time.store(SimulationClock::getInstance().getCurrentTime());
                    log_brief("[主函数：动力学模型] 开始更新动力学模型\n");
                    dynamicsModel->step(state, update_queue, bus, SimulationClock::getInstance(), aircraftConfig, forceModel);
                    log_brief("[主函数：动力学模型] 通知时钟步骤已完成\n");
                    SimulationClock::getInstance().notifyStepCompleted();
                    log_brief("[主函数：动力学模型] 动力学模型更新完成\n");
                }
                SimulationClock::getInstance().unregisterThread();
                log_brief("[主函数：动力学模型] 动力学模型线程已结束\n");
            });
            dynamics_thread_started = true;
        }
    };
    auto stop_dynamics = [&]() {
        if (dynamics_thread_started && dynamics_thread.joinable()) {
            dynamics_thread.join();
            log_brief("[主函数：动力学模型] 动力学模型已停止\n");
        }
    };
    auto start_simulation_control = [&]() {
        simulation_control_thread.start();
        log_brief("[主函数：仿真控制] 仿真控制线程已启动\n");
    };
    auto stop_simulation_control = [&]() {
        simulation_control_thread.stop();
        simulation_control_thread.join();
        log_brief("[主函数：仿真控制] 仿真控制线程已停止\n");
    };
    start_simulation_control();
    start_clock();
    start_state_manager();
    start_event_monitor();
    start_controller_manager();
    start_dynamics();
    start_data_recorder();
    while (state.simulation_running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    stop_data_recorder();
    stop_dynamics();
    stop_controller_manager();
    stop_event_monitor();
    stop_state_manager();
    stop_clock();
    stop_simulation_control();
    log_brief("========= 仿真结束 =========\n");
    return 0;
} 