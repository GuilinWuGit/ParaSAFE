/*********************************************************************
 * 设置控制台输出编码
 * 
 * 该文件定义了设置控制台输出编码的函数。
 * 
 * ********************************************************************/ 

#pragma once
#include <windows.h>
namespace ConsoleOutput {
    inline bool initialize() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        return true;
    }
} 