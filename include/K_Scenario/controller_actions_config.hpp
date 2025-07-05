/*
 * @file controller_actions_config.hpp
 * @brief 控制器动作配置头文件
 *
 * 本文件定义了ParaSAFE仿真系统中控制器动作的配置管理，负责解析和管理
 * 控制器动作的映射关系，支持从配置文件动态加载动作配置。
 *
 * 主要功能：
 *   - 定义控制器动作配置结构
 *   - 提供动作配置的加载和解析
 *   - 支持配置文件动态配置
 *   - 管理动作到控制器的映射关系
 */

#pragma once

// C++系统头文件
#include <string>           // 字符串类型，用于配置项名称和值
#include <map>              // 映射容器，存储配置键值对
#include <vector>           // 向量容器，存储配置列表
#include <fstream>          // 文件流，用于读取配置文件
#include <iostream>         // 输入输出流，用于配置加载和错误输出
#include <sstream>          // 字符串流，用于配置解析

// 控制器动作配置结构
struct ControllerActionConfig {
    std::string controller_name;           // 控制器名称
    std::map<std::string, std::string> state_settings;  // 状态设置 (变量名=值)
    std::string action_type;               // 动作类型 (CONTROLLER, MODE, STOP_ALL)
};

class ControllerActionsConfig {
private:
    static std::map<std::string, ControllerActionConfig> action_configs;
    static bool config_loaded;

public:
    // 从配置文件加载配置
    static void loadConfig(const std::string& filename = "controller_actions_config.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "[ControllerActionsConfig] 配置文件不存在，使用默认配置" << std::endl;
            loadDefaultConfig();
            return;
        }

        std::string line;
        int line_count = 0;
        while (std::getline(file, line)) {
            line_count++;
            
            // 跳过空行和注释行
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // 查找等号位置
            size_t equal_pos = line.find('=');
            if (equal_pos == std::string::npos) {
                std::cout << "[ControllerActionsConfig] 警告：第" << line_count << "行格式错误，缺少等号: " << line << std::endl;
                continue;
            }
            
            // 提取动作名和配置
            std::string action_name = line.substr(0, equal_pos);
            std::string config_str = line.substr(equal_pos + 1);
            
            // 去除前后空格
            action_name.erase(0, action_name.find_first_not_of(" \t"));
            action_name.erase(action_name.find_last_not_of(" \t") + 1);
            config_str.erase(0, config_str.find_first_not_of(" \t"));
            config_str.erase(config_str.find_last_not_of(" \t") + 1);
            
            // 解析配置
            ControllerActionConfig config = parseConfig(config_str);
            action_configs[action_name] = config;
            
            std::cout << "[ControllerActionsConfig] 加载动作配置: " << action_name 
                      << " -> " << config.controller_name << std::endl;
        }
        
        config_loaded = true;
        std::cout << "[ControllerActionsConfig] 配置文件加载完成，共加载 " 
                  << action_configs.size() << " 个动作配置" << std::endl;
    }

    // 解析配置字符串
    static ControllerActionConfig parseConfig(const std::string& config_str) {
        ControllerActionConfig config;
        
        // 查找逗号分隔符
        size_t comma_pos = config_str.find(',');
        if (comma_pos == std::string::npos) {
            // 没有状态设置
            config.controller_name = config_str;
            config.action_type = getActionType(config_str);
            return config;
        }
        
        // 提取控制器名称
        std::string controller_name = config_str.substr(0, comma_pos);
        controller_name.erase(0, controller_name.find_first_not_of(" \t"));
        controller_name.erase(controller_name.find_last_not_of(" \t") + 1);
        config.controller_name = controller_name;
        config.action_type = getActionType(controller_name);
        
        // 解析状态设置
        std::string state_settings_str = config_str.substr(comma_pos + 1);
        state_settings_str.erase(0, state_settings_str.find_first_not_of(" \t"));
        
        if (!state_settings_str.empty()) {
            // 处理多个状态设置（用分号分隔）
            std::istringstream iss(state_settings_str);
            std::string setting;
            while (std::getline(iss, setting, ';')) {
                size_t equal_pos = setting.find('=');
                if (equal_pos != std::string::npos) {
                    std::string var_name = setting.substr(0, equal_pos);
                    std::string value = setting.substr(equal_pos + 1);
                    
                    // 去除空格
                    var_name.erase(0, var_name.find_first_not_of(" \t"));
                    var_name.erase(var_name.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    config.state_settings[var_name] = value;
                }
            }
        }
        
        return config;
    }

    // 获取动作类型
    static std::string getActionType(const std::string& controller_name) {
        if (controller_name == "STOP_ALL") return "STOP_ALL";
        if (controller_name == "MODE") return "MODE";
        return "CONTROLLER";
    }

    // 获取动作配置
    static const ControllerActionConfig* getActionConfig(const std::string& action_name) {
        if (!config_loaded) {
            loadDefaultConfig();
        }
        
        auto it = action_configs.find(action_name);
        if (it != action_configs.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    // 加载默认配置
    static void loadDefaultConfig() {
        action_configs.clear();
        
        // 默认的控制器动作映射
        action_configs["START_THROTTLE_INCREASE"] = {"油门增加", {{"throttle_control_enabled", "true"}}, "CONTROLLER"};
        action_configs["STOP_THROTTLE_INCREASE"] = {"油门增加", {{"throttle_control_enabled", "false"}}, "CONTROLLER"};
        action_configs["START_THROTTLE_DECREASE"] = {"油门减少", {{"throttle_control_enabled", "true"}}, "CONTROLLER"};
        action_configs["STOP_THROTTLE_DECREASE"] = {"油门减少", {{"throttle_control_enabled", "false"}}, "CONTROLLER"};
        action_configs["START_BRAKE"] = {"刹车", {{"cruise_control_enabled", "false"}, {"brake_control_enabled", "true"}}, "CONTROLLER"};
        action_configs["STOP_BRAKE"] = {"刹车", {{"brake_control_enabled", "false"}}, "CONTROLLER"};
        action_configs["START_CRUISE"] = {"跑道巡航", {{"cruise_control_enabled", "true"}}, "CONTROLLER"};
        action_configs["STOP_CRUISE"] = {"跑道巡航", {{"cruise_control_enabled", "false"}}, "CONTROLLER"};
        action_configs["START_PITCH_CONTROL"] = {"俯仰角保持", {{"pitch_control_enabled", "true"}}, "CONTROLLER"};
        action_configs["STOP_PITCH_CONTROL"] = {"俯仰角保持", {{"pitch_control_enabled", "false"}}, "CONTROLLER"};
        action_configs["SET_PITCH_ANGLE"] = {"俯仰角保持", {}, "PITCH_SETTING"};
        action_configs["STOP_ALL_CONTROLLERS"] = {"STOP_ALL", {}, "STOP_ALL"};
        action_configs["SWITCH_TO_AUTO_MODE"] = {"MODE", {{"flight_mode", "AUTO"}}, "MODE"};
        action_configs["SWITCH_TO_MANUAL_MODE"] = {"MODE", {{"flight_mode", "MANUAL"}}, "MODE"};
        action_configs["SWITCH_TO_SEMI_AUTO_MODE"] = {"MODE", {{"flight_mode", "SEMI_AUTO"}}, "MODE"};
        
        config_loaded = true;
        std::cout << "[ControllerActionsConfig] 已加载默认配置" << std::endl;
    }

    // 打印所有配置
    static void printAllConfigs() {
        std::cout << "[ControllerActionsConfig] 当前配置:" << std::endl;
        for (const auto& [action_name, config] : action_configs) {
            std::cout << "  " << action_name << " -> " << config.controller_name 
                      << " (" << config.action_type << ")" << std::endl;
            for (const auto& [var, value] : config.state_settings) {
                std::cout << "    " << var << " = " << value << std::endl;
            }
        }
    }
};

// 静态成员初始化
inline std::map<std::string, ControllerActionConfig> ControllerActionsConfig::action_configs;
inline bool ControllerActionsConfig::config_loaded = false;

 