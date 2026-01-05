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
