#include "tianbot_core_ros2/omni.h"
#include "tianbot_core_ros2/protocol.h"

void TianbotOmni::velocityCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    uint16_t len;
    vector<uint8_t> buf;

    struct twist twist;
    uint8_t *out = (uint8_t *)&twist;
    twist.linear.x = msg->linear.x;
    twist.linear.y = msg->linear.y;
    twist.linear.z = msg->linear.z;
    twist.angular.x = msg->angular.x;
    twist.angular.y = msg->angular.y;
    twist.angular.z = msg->angular.z;

    buildCmd(buf, PACK_TYPE_CMD_VEL, (uint8_t *)&twist, sizeof(twist));
    serial_.send(&buf[0], buf.size());

    heartbeat_timer_->cancel();
    heartbeat_timer_->reset();
}

TianbotOmni::TianbotOmni() : TianbotChasis()
{
    cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 1, std::bind(&TianbotOmni::velocityCallback, this, std::placeholders::_1));
}
