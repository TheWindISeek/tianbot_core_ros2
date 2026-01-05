cmake_minimum_required(VERSION 3.8)
project(tianbot_core_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
# ackermann_msgs is optional - only needed for ackermann chassis type
find_package(ackermann_msgs QUIET)
if(ackermann_msgs_FOUND)
  add_definitions(-DHAVE_ACKERMANN_MSGS)
  message(STATUS "Found ackermann_msgs - Ackermann chassis support enabled")
else()
  message(WARNING "ackermann_msgs not found - Ackermann chassis support disabled")
  message(STATUS "To enable Ackermann support, install ackermann_msgs:")
  message(STATUS "  git clone https://github.com/ros-drivers/ackermann_msgs.git -b ros2")
  message(STATUS "  colcon build --packages-select ackermann_msgs")
endif()
find_package(rosidl_default_generators REQUIRED)

# Generate services
rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/DebugCmd.srv"
  DEPENDENCIES std_msgs
)

# Export dependencies
ament_export_dependencies(
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  std_msgs
  tf2
  tf2_ros
  rosidl_default_runtime
)
if(ackermann_msgs_FOUND)
  ament_export_dependencies(ackermann_msgs)
endif()

###########
## Build ##
###########

include_directories(
  include
  ${CMAKE_CURRENT_BINARY_DIR}/rosidl_generator_cpp
)

# Declare executable
add_executable(${PROJECT_NAME}_node 
  src/core.cpp 
  src/serial.cpp 
  src/main.cpp 
  src/chassis.cpp 
  src/omni.cpp 
  src/ackermann.cpp 
  src/differential.cpp 
  src/protocol.cpp
)

# Add dependencies
ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  geometry_msgs
  nav_msgs
  sensor_msgs
  std_msgs
  tf2
  tf2_ros
)
if(ackermann_msgs_FOUND)
  ament_target_dependencies(${PROJECT_NAME}_node ackermann_msgs)
endif()

# Link against generated service types
rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME}_node "${cpp_typesupport_target}")

target_link_libraries(${PROJECT_NAME}_node
  pthread
)

#############
## Install ##
#############

install(TARGETS ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

install(DIRECTORY param/
  DESTINATION share/${PROJECT_NAME}/param
)

install(PROGRAMS scripts/param_set.py
  DESTINATION lib/${PROJECT_NAME}
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
