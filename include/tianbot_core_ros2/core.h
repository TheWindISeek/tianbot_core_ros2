// BSD 3-Clause License
//
// Copyright (c) 2019-2020, TIANBOT
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Modified by JeffreySharp (2026) - ROS2 port from ROS1

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
