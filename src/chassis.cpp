#include "tianbot_core_ros2/chassis.h"
#include "tianbot_core_ros2/protocol.h"
#include <tf2/LinearMath/Quaternion.h>

void TianbotChasis::tianbotDataProc(unsigned char *buf, int len)
{
    struct protocol_pack *p = (struct protocol_pack *)buf;
    switch (p->pack_type)
    {
    case PACK_TYPE_ODOM_RESPONSE:
        if (sizeof(struct odom) == p->len - 2)
        {
            nav_msgs::msg::Odometry odom_msg;
            struct odom *pOdom = (struct odom *)(p->data);
            rclcpp::Time current_time = this->now();
            odom_msg.header.stamp = current_time;
            odom_msg.header.frame_id = odom_frame_;

            odom_msg.pose.pose.position.x = pOdom->pose.point.x;
            odom_msg.pose.pose.position.y = pOdom->pose.point.y;
            odom_msg.pose.pose.position.z = pOdom->pose.point.z;
            
            // Convert yaw to quaternion
            tf2::Quaternion q;
            q.setRPY(0, 0, pOdom->pose.yaw);
            odom_msg.pose.pose.orientation.x = q.x();
            odom_msg.pose.pose.orientation.y = q.y();
            odom_msg.pose.pose.orientation.z = q.z();
            odom_msg.pose.pose.orientation.w = q.w();
            
            //set the velocity
            odom_msg.child_frame_id = base_frame_;
            odom_msg.twist.twist.linear.x = pOdom->twist.linear.x;
            odom_msg.twist.twist.linear.y = pOdom->twist.linear.y;
            odom_msg.twist.twist.linear.z = pOdom->twist.linear.z;
            odom_msg.twist.twist.angular.x = pOdom->twist.angular.x;
            odom_msg.twist.twist.angular.y = pOdom->twist.angular.y;
            odom_msg.twist.twist.angular.z = pOdom->twist.angular.z;
            //publish the message
            odom_pub_->publish(odom_msg);
            if (publish_tf_)
            {
                odom_tf_.header.stamp = current_time;
                odom_tf_.header.frame_id = odom_frame_;
                odom_tf_.child_frame_id = base_frame_;
                odom_tf_.transform.translation.x = pOdom->pose.point.x;
                odom_tf_.transform.translation.y = pOdom->pose.point.y;
                odom_tf_.transform.translation.z = pOdom->pose.point.z;

                odom_tf_.transform.rotation = odom_msg.pose.pose.orientation;
                tf_broadcaster_->sendTransform(odom_tf_);
            }
        }
        break;

    case PACK_TYPE_UWB_RESPONSE:
        if (sizeof(struct uwb) == p->len - 2)
        {
            geometry_msgs::msg::Pose2D pose2d_msg;
            struct uwb *pUwb = (struct uwb *)(p->data);
            pose2d_msg.x = pUwb->x_m;
            pose2d_msg.y = pUwb->y_m;
            pose2d_msg.theta = pUwb->yaw;
            uwb_pub_->publish(pose2d_msg);
        }
        break;

    case PACK_TYPE_HEART_BEAT_RESPONSE:
        // Heartbeat response received, communication is alive
        communication_timer_->reset();
        break;

    case PACK_TYPE_IMU_REPONSE:
        if (sizeof(struct imu_feedback) == p->len - 2)
        {
            sensor_msgs::msg::Imu imu_msg;
            struct imu_feedback *pImu = (struct imu_feedback *)(p->data);

            rclcpp::Time current_time = this->now();
            imu_msg.header.stamp = current_time;
            imu_msg.header.frame_id = imu_frame_;
            imu_msg.orientation.x = pImu->quat.x;
            imu_msg.orientation.y = pImu->quat.y;
            imu_msg.orientation.z = pImu->quat.z;
            imu_msg.orientation.w = pImu->quat.w;
            imu_msg.angular_velocity.x = pImu->angular_vel.x;
            imu_msg.angular_velocity.y = pImu->angular_vel.y;
            imu_msg.angular_velocity.z = pImu->angular_vel.z;
            imu_msg.linear_acceleration.x = pImu->linear_acc.x;
            imu_msg.linear_acceleration.y = pImu->linear_acc.y;
            imu_msg.linear_acceleration.z = pImu->linear_acc.z;
            imu_pub_->publish(imu_msg);
        }
        break;

    case PACK_TYPE_DEBUG_RESPONSE:
        {
            std_msgs::msg::String debug_msg;
            p->data[p->len-2] = '\0';
            debug_msg.data = (char *)(p->data);
            debugResultStr_ = (char *)(p->data);
            debugResultFlag_ = true;
            debug_result_pub_->publish(debug_msg);
        }
        break;

    default:
        break;
    }
}

TianbotChasis::TianbotChasis() : TianbotCore()
{
    this->declare_parameter<std::string>("base_frame", DEFAULT_BASE_FRAME);
    this->declare_parameter<std::string>("odom_frame", DEFAULT_ODOM_FRAME);
    this->declare_parameter<std::string>("imu_frame", DEFAULT_IMU_FRAME);
    this->declare_parameter<bool>("publish_tf", DEFAULT_PUBLISH_TF);
    
    this->get_parameter("base_frame", base_frame_);
    this->get_parameter("odom_frame", odom_frame_);
    this->get_parameter("imu_frame", imu_frame_);
    this->get_parameter("publish_tf", publish_tf_);

    odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 1);
    imu_pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu", 1);
    uwb_pub_ = this->create_publisher<geometry_msgs::msg::Pose2D>("uwb", 1);

    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    odom_tf_.header.frame_id = odom_frame_;
    odom_tf_.child_frame_id = base_frame_;
}
