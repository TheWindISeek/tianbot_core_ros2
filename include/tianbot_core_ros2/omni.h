#ifndef __OMNI_H__
#define __OMNI_H__

#include "rclcpp/rclcpp.hpp"
#include "tianbot_core_ros2/chassis.h"
#include "geometry_msgs/msg/twist.hpp"
#include <memory>

class TianbotOmni : public TianbotChasis{
public:
    TianbotOmni();
private:
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
    void velocityCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
};

#endif
