#pragma once
#include <string>

/**
 * @brief 跑道模型接口
 */
class IRunwayModel {
public:
    virtual ~IRunwayModel() = default;
    // 跑道名称
    virtual std::string getName() const = 0;
    // 跑道长度（米）
    virtual double getLength() const = 0;
    // 跑道宽度（米）
    virtual double getWidth() const = 0;
    // 跑道摩擦系数
    virtual double getFrictionCoefficient() const = 0;
};

/**
 * @brief 简单直线跑道实现
 */
class SimpleRunway : public IRunwayModel {
public:
    SimpleRunway(const std::string& name, double length, double width, double friction)
        : name_(name), length_(length), width_(width), friction_(friction) {}
    std::string getName() const override { return name_; }
    double getLength() const override { return length_; }
    double getWidth() const override { return width_; }
    double getFrictionCoefficient() const override { return friction_; }
private:
    std::string name_;
    double length_;
    double width_;
    double friction_;
}; 