#ifndef __ACKERMANN_H__
#define __ACKERMANN_H__

#include "rclcpp/rclcpp.hpp"
#include "tianbot_core_ros2/chassis.h"
#include "ackermann_msgs/msg/ackermann_drive.hpp"
#include <memory>

class TianbotAckermann : public TianbotChasis
{
public:
    TianbotAckermann();

private:
    rclcpp::Subscription<ackermann_msgs::msg::AckermannDrive>::SharedPtr ackermann_sub_;
    void ackermannCallback(const ackermann_msgs::msg::AckermannDrive::SharedPtr msg);
};

#endif
