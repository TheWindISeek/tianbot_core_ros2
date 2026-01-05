#ifndef __CORE_H__
#define __CORE_H__

#include "rclcpp/rclcpp.hpp"
#include "tianbot_core_ros2/serial.h"
#include "string.h"
#include "std_msgs/msg/string.hpp"
#include "tianbot_core_ros2/srv/debug_cmd.hpp"
#include <string>
#include <memory>
#include <chrono>
#include <functional>

#define DEFAULT_SERIAL_DEVICE "/dev/ttyUSB0"
#define DEFAULT_SERIAL_BAUDRATE 460800
#define DEFAULT_TYPE "omni"
#define DEFAULT_TYPE_VERIFY true

using namespace std;

class TianbotCore : public rclcpp::Node
{
public:
    Serial serial_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr debug_result_pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr debug_cmd_sub_;
    rclcpp::TimerBase::SharedPtr heartbeat_timer_;
    rclcpp::TimerBase::SharedPtr communication_timer_;

    bool debugResultFlag_;
    string debugResultStr_;

    TianbotCore();
    void checkDevType(void);
    virtual void tianbotDataProc(unsigned char *buf, int len) = 0;

private:
    void serialDataProc(uint8_t *data, unsigned int data_len);
    void heartCallback(void);
    void communicationErrorCallback(void);
    void debugCmdCallback(const std_msgs::msg::String::SharedPtr msg);
    void debugCmdSrv(
        const std::shared_ptr<tianbot_core_ros2::srv::DebugCmd::Request> req,
        std::shared_ptr<tianbot_core_ros2::srv::DebugCmd::Response> res);
    rclcpp::Service<tianbot_core_ros2::srv::DebugCmd>::SharedPtr param_set_;
};

#endif
