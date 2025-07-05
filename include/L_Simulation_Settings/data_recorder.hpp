#pragma once
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>
#include <mutex>
#include "../K_Scenario/shared_state.hpp"
#include <stdexcept>
#include <map>
#include <chrono>
#include <ctime>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <functional>
#include <any>
#include "logger.hpp"
#include "thread_name_util.hpp"

struct SimulationData {
    double time;
    double velocity;
    double position;
    double throttle;
    double brake;
};

class FileLogger {
private:
    std::string filename_;
    std::mutex mtx_;
    std::ofstream data_file_;
    double last_time_ = -1.0; // 上一次记录的时间戳

public:
    FileLogger(const std::string& filename)
        : filename_(filename) {
        // 先清空文件并写入表头
        data_file_.open("output/data.csv", std::ios::out | std::ios::trunc);
        if (data_file_.is_open()) {
            data_file_ << std::left
                       << std::setw(12) << "time"
                       << std::setw(12) << "position"
                       << std::setw(12) << "velocity"
                       << std::setw(12) << "acc"
                       << std::setw(12) << "throttle"
                       << std::setw(12) << "brake"
                       << std::setw(12) << "thrust"
                       << std::setw(12) << "drag"
                       << std::setw(12) << "brake_force"
                       << std::endl;
            data_file_.flush();
            data_file_.close(); // 关闭文件，避免与后续数据输出冲突
            log_detail("[FileLogger] CSV表头已写入: time, position, velocity, acc, throttle, brake, thrust, drag, brake_force\n");
        } else {
            log_detail("[FileLogger] 错误：无法打开output/data.csv文件\n");
        }
    }

    ~FileLogger() {
        if (data_file_.is_open()) data_file_.close();
    }

    void recordData(const std::map<std::string, double>& data) {
        std::lock_guard<std::mutex> lock(mtx_);
        double current_time = data.at("time");

        // 验证时间戳，必须严格递增
        if (current_time <= last_time_) {
            log_detail("[FileLogger] 警告：检测到重复或非递增的时间戳。当前时间: " + std::to_string(current_time) +
                              ", 上次时间: " + std::to_string(last_time_) + ". 已跳过此条数据。\n");
            return; // 跳过，不记录
        }
        
        last_time_ = current_time; // 更新时间戳

        // 重新打开文件，追加模式
        data_file_.open("output/data.csv", std::ios::out | std::ios::app);
        if (data_file_.is_open()) {
            data_file_ << std::left << std::fixed
                       << std::setprecision(2) // 默认精度为2
                       << std::setw(12) << current_time
                       << std::setw(12) << data.at("position")
                       << std::setw(12) << data.at("velocity")
                       << std::setw(12) << data.at("acceleration")
                       << std::setprecision(4) // throttle精度为3
                       << std::setw(12) << data.at("throttle")
                       << std::setprecision(2) // 恢复默认精度为2
                       << std::setw(12) << data.at("brake")
                       << std::setw(12) << data.at("thrust")
                       << std::setw(12) << data.at("drag")
                       << std::setw(12) << data.at("brake_force")
                       << std::endl;
            data_file_.flush();
            data_file_.close(); // 写入后立即关闭文件
        } else {
            log_detail("[FileLogger] 错误：无法打开output/data.csv文件进行写入\n");
        }
    }
};

// 数据输出线程，严格与仿真时钟同步
class DataRecorderThread {
public:
    DataRecorderThread(SharedStateSpace& state, SimulationClock& clock, FileLogger& logger)
        : state_(state), clock_(clock), logger_(logger), running_(false) {}

    void start() {
        if (!running_) {
            running_ = true;
            thread_ = std::thread(&DataRecorderThread::run, this);
        }
    }
    void stop() {
        if (running_) {
            running_ = false;
            if (thread_.joinable()) thread_.join();
        }
    }
    void join() {
        if (thread_.joinable()) thread_.join();
    }
    bool isRunning() const { return running_.load(); }

private:
    void run() {
        ThreadNaming::set_current_thread_name("DataRecorder");
        clock_.registerThread();
        size_t output_count = 0;
        size_t current_step = 0; // 记录当前线程处理到的步数

        // 先输出一次初始状态，time=0.00
        {
            double t = 0.0;
            output_count++;
            log_detail("[DataRecorder] 初始输出 步数=0 current_time=0.00 输出次数=" + std::to_string(output_count) + "\n");
            std::map<std::string, double> data = {
                {"time", t},
                {"position", state_.position.load()},
                {"velocity", state_.velocity.load()},
                {"acceleration", state_.acceleration.load()},
                {"throttle", state_.throttle.load()},
                {"brake", state_.brake.load()},
                {"thrust", state_.thrust.load()},
                {"drag", state_.drag_force.load()},
                {"brake_force", state_.brake_force.load()}
            };
            logger_.recordData(data);
            log_detail("[DataRecorder] 初始输出完成 步数=0 current_time=0.00 输出次数=" + std::to_string(output_count) + "\n");
        }

        // 从0.01秒开始，按时间步长递增记录数据
        double next_time = 0.01; // 下一个要记录的时间点
        
        while (running_ && clock_.isRunning()) {
            clock_.waitForNextStep(current_step);
            current_step = clock_.getStepCount(); // 更新为最新的时钟步数

            double current_clock_time = clock_.getCurrentTime();
            
            // 如果时钟时间已经超过了下一个要记录的时间点，就记录数据
            if (current_clock_time >= next_time) {
                output_count++;
                log_detail("[DataRecorder] 线程(" + ThreadNaming::get_current_thread_name() + ") " +
                                  "步数=" + std::to_string(clock_.getStepCount()) +
                                  " current_time=" + std::to_string(next_time) +
                                  " 输出次数=" + std::to_string(output_count) + "\n");
                // 采集数据
                std::map<std::string, double> data = {
                    {"time", next_time},
                    {"position", state_.position.load()},
                    {"velocity", state_.velocity.load()},
                    {"acceleration", state_.acceleration.load()},
                    {"throttle", state_.throttle.load()},
                    {"brake", state_.brake.load()},
                    {"thrust", state_.thrust.load()},
                    {"drag", state_.drag_force.load()},
                    {"brake_force", state_.brake_force.load()}
                };
                logger_.recordData(data);
                log_detail("[DataRecorder] 输出完成 步数=" + std::to_string(clock_.getStepCount()) +
                                  " current_time=" + std::to_string(next_time) +
                                  " 输出次数=" + std::to_string(output_count) + "\n");
                
                // 更新下一个要记录的时间点
                next_time += 0.01; // 按时间步长递增
            }
            
            clock_.notifyStepCompleted();
        }
        clock_.unregisterThread();
    }
    SharedStateSpace& state_;
    SimulationClock& clock_;
    FileLogger& logger_;
    std::atomic<bool> running_;
    std::thread thread_;
};