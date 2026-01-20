from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'serial_port',
            default_value='/dev/tianbot_racecar',
            description='Serial port device'
        ),
        DeclareLaunchArgument(
            'serial_baudrate',
            # default_value='115200',
            default_value='460800',
            description='Serial baudrate'
        ),
        DeclareLaunchArgument(
            'type',
            default_value='ackermann',
            description='Robot type: omni, diff, ackermann'
        ),
        DeclareLaunchArgument(
            'type_verify',
            default_value='true',
            description='Verify device type'
        ),
        DeclareLaunchArgument(
            'publish_tf',
            default_value='true',
            description='Publish TF transforms'
        ),
        DeclareLaunchArgument(
            'yaw_offset_deg',
            default_value='0.0',
            description='Odom yaw offset in degrees (for steering calibration)'
        ),
        Node(
            package='tianbot_core_ros2',
            executable='tianbot_core_ros2_node',
            name='tianbot_core',
            output='screen',
            parameters=[{
                'serial_port': LaunchConfiguration('serial_port'),
                'serial_baudrate': LaunchConfiguration('serial_baudrate'),
                'type': LaunchConfiguration('type'),
                'type_verify': LaunchConfiguration('type_verify'),
                'publish_tf': LaunchConfiguration('publish_tf'),
                'yaw_offset_deg': LaunchConfiguration('yaw_offset_deg'),
            }]
        ),
    ])

