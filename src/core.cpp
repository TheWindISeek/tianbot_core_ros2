#include "tianbot_core_ros2/core.h"
#include "tianbot_core_ros2/protocol.h"
#include <vector>
#include <stdint.h>

void TianbotCore::serialDataProc(uint8_t *data, unsigned int data_len)
{
    static uint8_t state = 0;
    uint8_t *p = data;
    static vector<uint8_t> recv_msg;
    static uint32_t len;
    uint32_t j;

    while (data_len != 0)
    {
        switch (state)
        {
        case 0:
            if (*p == (PROTOCOL_HEAD & 0xFF))
            {
                recv_msg.clear();
                recv_msg.push_back(PROTOCOL_HEAD & 0xFF);
                state = 1;
            }
            p++;
            data_len--;
            break;

        case 1:
            if (*p == ((PROTOCOL_HEAD >> 8) & 0xFF))
            {
                recv_msg.push_back(((PROTOCOL_HEAD >> 8) & 0xFF));
                p++;
                data_len--;
                state = 2;
            }
            else
            {
                state = 0;
            }
            break;

        case 2: // len
            recv_msg.push_back(*p);
            len = *p;
            p++;
            data_len--;
            state = 3;
            break;

        case 3: // len
            recv_msg.push_back(*p);
            len += (*p) * 256;
            if (len > 1024 * 10)
            {
                state = 0;
                break;
            }
            p++;
            data_len--;
            state = 4;
            break;

        case 4: // pack_type
            recv_msg.push_back(*p);
            p++;
            data_len--;
            len--;
            state = 5;
            break;

        case 5: // pack_type
            recv_msg.push_back(*p);
            p++;
            data_len--;
            len--;
            state = 6;
            break;

        case 6: //
            if (len--)
            {
                recv_msg.push_back(*p);
                p++;
                data_len--;
            }
            else
            {
                state = 7;
            }
            break;

        case 7:
        {
            int i;
            uint8_t bcc = 0;
            uint8_t received_bcc = *p;
            recv_msg.push_back(received_bcc);  // This is the BCC byte
            p++;
            data_len--;
            state = 0;

            // BCC is calculated from byte 4 (after head and len) to the byte before BCC
            // recv_msg structure: [head_low, head_high, len_low, len_high, pack_type_low, pack_type_high, data..., bcc]
            // BCC should be calculated from index 4 to size-1 (excluding the BCC byte itself)
            // This matches buildCmd: for (i = 4; i < buf.size(); i++) where buf.size() doesn't include BCC yet
            for (i = 4; i < recv_msg.size() - 1; i++)
            {
                bcc ^= recv_msg[i];
            }

            if (bcc == received_bcc)
            {
                tianbotDataProc(&recv_msg[0], recv_msg.size()); // process recv msg
                communication_timer_->reset();                    // restart timer for communication timeout
            }
            else
            {
                // Only log BCC errors at DEBUG level to reduce noise, but keep them for debugging
                RCLCPP_DEBUG(this->get_logger(), "BCC error: calculated=0x%02x, received=0x%02x, msg_size=%zu", 
                            bcc, received_bcc, recv_msg.size());
            }
            state = 0;
        }
        break;

        default:
            state = 0;
            break;
        }
    }
}

void TianbotCore::communicationErrorCallback(void)
{
    // This is called when no valid data packet received within timeout period
    // If communication is working (commands are being executed), this might just be
    // a temporary delay in heartbeat response. Lower to WARNING level.
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                        "No communication response received - check if device is responding");
}

void TianbotCore::heartCallback(void)
{
    vector<uint8_t> buf;
    uint16_t dummy = 0;

    buildCmd(buf, PACK_TYPE_HEART_BEAT, (uint8_t *)&dummy, sizeof(dummy));
    int ret = serial_.send(&buf[0], buf.size());
    
    // If heartbeat sent successfully, reset communication timer
    // This indicates communication link is active even if response is delayed
    if (ret == 0)
    {
        communication_timer_->reset();
    }
    
    if (ret != 0)
    {
        std::string param_serial_port;
        int32_t param_serial_baudrate;
        this->get_parameter("serial_port", param_serial_port);
        this->get_parameter("serial_baudrate", param_serial_baudrate);
        heartbeat_timer_->cancel();
        communication_timer_->cancel();
        
        auto recv_cb = std::bind(&TianbotCore::serialDataProc, this, std::placeholders::_1, std::placeholders::_2);
        while (serial_.open(param_serial_port.c_str(), param_serial_baudrate, 0, 8, 1, 'N', recv_cb) != true)
        {
            RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Device %s disconnected", param_serial_port.c_str());
            rclcpp::sleep_for(std::chrono::milliseconds(500));
        }
        RCLCPP_INFO(this->get_logger(), "Device %s connected", param_serial_port.c_str());
        heartbeat_timer_->reset();
        communication_timer_->reset();
    }
}

void TianbotCore::debugCmdCallback(const std_msgs::msg::String::SharedPtr msg)
{
    vector<uint8_t> buf;
    buildCmd(buf, PACK_TYPE_DEBUG, (uint8_t *)msg->data.c_str(), msg->data.length());
    serial_.send(&buf[0], buf.size());
}

void TianbotCore::debugCmdSrv(
    const std::shared_ptr<tianbot_core_ros2::srv::DebugCmd::Request> req,
    std::shared_ptr<tianbot_core_ros2::srv::DebugCmd::Response> res)
{
    vector<uint8_t> buf;
    debugResultFlag_ = false;
    uint32_t count = 200;
    buildCmd(buf, PACK_TYPE_DEBUG, (uint8_t *)req->cmd.c_str(), req->cmd.length());
    serial_.send(&buf[0], buf.size());
    if (req->cmd == "reset")
    {
        res->result = "reset";
        return;
    }
    else if (req->cmd == "param save")
    {
        count = 2000;
    }
    while (count-- && !debugResultFlag_)
    {
        rclcpp::sleep_for(std::chrono::milliseconds(1));
    }
    if (debugResultFlag_)
    {
        res->result = debugResultStr_;
    }
    else
    {
        res->result = "";
    }
}

void TianbotCore::checkDevType(void)
{
    string type_keyword_list[] = {"base_type: ", "end"};
    string type;
    string dev_param;
    string dev_type;
    string::size_type start;
    string::size_type end;

    string cmd = "param get";
    vector<uint8_t> buf;
    uint32_t count;
    uint32_t retry;

    for (retry = 0; retry < 5; retry++)
    {
        debugResultFlag_ = false;
        count = 300;
        buf.clear();
        buildCmd(buf, PACK_TYPE_DEBUG, (uint8_t *)cmd.c_str(), cmd.length());
        serial_.send(&buf[0], buf.size());

        while (count-- && !debugResultFlag_)
        {
            rclcpp::sleep_for(std::chrono::milliseconds(1));
        }
        if (debugResultFlag_)
        {
            dev_param = debugResultStr_;
            break;
        }
        else
        {
            RCLCPP_INFO(this->get_logger(), "Get Device type failed, retry after 1s ...");
            rclcpp::sleep_for(std::chrono::seconds(1));
        }
    }
    if (retry == 5)
    {
        RCLCPP_ERROR(this->get_logger(), "No valid device type found");
        return;
    }
    for (int i = 0; type_keyword_list[i] != "end"; i++)
    {
        start = dev_param.find(type_keyword_list[i]);
        if (start != dev_param.npos)
        {
            start += type_keyword_list[i].length();
            end = dev_param.find("\r\n", start);
            if (end == dev_param.npos)
            {
                end = dev_param.length();
            }
            dev_type = dev_param.substr(start, end - start);
            RCLCPP_INFO(this->get_logger(), "Get device type [%s]", dev_type.c_str());
            this->get_parameter("type", type);
            if (dev_type == "omni" || dev_type == "mecanum")
            {
                dev_type = "omni";
            }
            if (type == dev_type)
            {
                RCLCPP_INFO(this->get_logger(), "Device type match");
            }
            else
            {
                RCLCPP_ERROR(this->get_logger(), "Device type mismatch, set [%s] get [%s]", type.c_str(), dev_type.c_str());
            }
            return;
        }
    }
    RCLCPP_ERROR(this->get_logger(), "No valid device type found");
}

TianbotCore::TianbotCore() : Node("tianbot_core")
{
    std::string param_serial_port;
    int32_t param_serial_baudrate;
    
    // Declare all parameters that might be used
    this->declare_parameter<std::string>("serial_port", DEFAULT_SERIAL_DEVICE);
    this->declare_parameter<int>("serial_baudrate", DEFAULT_SERIAL_BAUDRATE);
    this->declare_parameter<std::string>("type", DEFAULT_TYPE);
    this->declare_parameter<bool>("type_verify", DEFAULT_TYPE_VERIFY);
    
    this->get_parameter("serial_port", param_serial_port);
    this->get_parameter("serial_baudrate", param_serial_baudrate);
    
    debug_result_pub_ = this->create_publisher<std_msgs::msg::String>("debug_result", 1);
    debug_cmd_sub_ = this->create_subscription<std_msgs::msg::String>(
        "debug_cmd", 1, std::bind(&TianbotCore::debugCmdCallback, this, std::placeholders::_1));
    param_set_ = this->create_service<tianbot_core_ros2::srv::DebugCmd>(
        "debug_cmd_srv", std::bind(&TianbotCore::debugCmdSrv, this, std::placeholders::_1, std::placeholders::_2));
    
    heartbeat_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(200), std::bind(&TianbotCore::heartCallback, this));
    // Communication timeout should be longer than heartbeat interval
    // Set to 2 seconds to allow for heartbeat response delay and device processing time
    communication_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(2000), std::bind(&TianbotCore::communicationErrorCallback, this));
    
    heartbeat_timer_->cancel();
    communication_timer_->cancel();
    
    auto recv_cb = std::bind(&TianbotCore::serialDataProc, this, std::placeholders::_1, std::placeholders::_2);
    while (serial_.open(param_serial_port.c_str(), param_serial_baudrate, 0, 8, 1, 'N', recv_cb) != true)
    {
        RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 5000, "Device %s connect failed", param_serial_port.c_str());
        rclcpp::sleep_for(std::chrono::milliseconds(500));
    }
    RCLCPP_INFO(this->get_logger(), "Device %s connect successfully", param_serial_port.c_str());
    heartbeat_timer_->reset();
    communication_timer_->reset();
}
