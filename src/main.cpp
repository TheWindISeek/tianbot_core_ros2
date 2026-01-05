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

#include "rclcpp/rclcpp.hpp"
#include "tianbot_core_ros2/core.h"
#include "string.h"
#include "stdint.h"
#include "tianbot_core_ros2/omni.h"
#include "tianbot_core_ros2/ackermann.h"
#include "tianbot_core_ros2/differential.h"
#include <sstream>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
    string type;
    bool type_verify;
    TianbotCore *core = nullptr;

    rclcpp::init(argc, argv);

    // Create a temporary node to get parameters before creating the actual node
    // This is needed because we need to know the type to create the correct node type
    std::ostringstream temp_node_name;
    temp_node_name << "temp_param_node_" << getpid();
    auto temp_node = rclcpp::Node::make_shared(temp_node_name.str());
    temp_node->declare_parameter<std::string>("type", DEFAULT_TYPE);
    temp_node->declare_parameter<bool>("type_verify", DEFAULT_TYPE_VERIFY);
    temp_node->get_parameter("type", type);
    temp_node->get_parameter("type_verify", type_verify);
    temp_node.reset();

    if (type == "omni")
    {
        core = new TianbotOmni();
    }
    else if (type == "ackermann")
    {
        core = new TianbotAckermann();
    }
    else if (type == "diff")
    {
        core = new TianbotDifferential();
    }
    else if (type == "arm")
    {
        // Not implemented yet
    }
    
    if (core == nullptr)
    {
        RCLCPP_ERROR(rclcpp::get_logger("tianbot_core"), "Invalid type: %s", type.c_str());
        rclcpp::shutdown();
        return -1;
    }
    
    if (type_verify)
    {
        core->checkDevType();
    }
    
    // Convert to shared_ptr for ROS2 spin
    std::shared_ptr<TianbotCore> core_ptr(core);
    rclcpp::spin(core_ptr);
    rclcpp::shutdown();
    
    return 0;
}
