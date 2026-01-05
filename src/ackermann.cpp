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
