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

#include "tianbot_core_ros2/ackermann.h"
#include "tianbot_core_ros2/protocol.h"

void TianbotAckermann::ackermannCallback(const ackermann_msgs::msg::AckermannDrive::SharedPtr msg)
{
    vector<uint8_t> buf;
    struct ackermann_cmd ackermann_cmd;
    uint8_t *out = (uint8_t *)&ackermann_cmd;

    ackermann_cmd.steering_angle = msg->steering_angle;
    ackermann_cmd.speed = msg->speed;

    RCLCPP_DEBUG(this->get_logger(), "Received ackermann cmd: steering=%.3f, speed=%.3f", 
                ackermann_cmd.steering_angle, ackermann_cmd.speed);

    buildCmd(buf, PACK_TYPE_ACKMAN_VEL, (uint8_t *)&ackermann_cmd, sizeof(ackermann_cmd));
    int ret = serial_.send(&buf[0], buf.size());
    
    if (ret == 0)
    {
        RCLCPP_DEBUG(this->get_logger(), "Ackermann command sent successfully");
    }
    else
    {
        RCLCPP_WARN(this->get_logger(), "Failed to send ackermann command, ret=%d", ret);
    }

    heartbeat_timer_->cancel();
    heartbeat_timer_->reset();
}

TianbotAckermann::TianbotAckermann() : TianbotChasis()
{
    ackermann_sub_ = this->create_subscription<ackermann_msgs::msg::AckermannDrive>(
        "ackermann_cmd", 5, std::bind(&TianbotAckermann::ackermannCallback, this, std::placeholders::_1));
}
