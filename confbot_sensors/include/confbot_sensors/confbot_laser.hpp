#ifndef CONFBOT_SENSORS__CONFBOT_LASER_HPP_
#define CONFBOT_SENSORS__CONFBOT_LASER_HPP_

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <cmath>

#include "lifecycle_msgs/msg/transition.hpp"

#include "rclcpp/rclcpp.hpp"

#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"

#include "rcutils/logging_macros.h"

#include "sensor_msgs/msg/laser_scan.hpp"

#define DEG2RAD M_PI / 180.0

using namespace std::chrono_literals;

namespace confbot_sensors
{

class ConfbotLaser : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit ConfbotLaser(const std::string & node_name, bool intra_process_comms = false)
  : rclcpp_lifecycle::LifecycleNode(node_name, "", intra_process_comms)
  {}

  void
  publish()
  {
    static auto distance = 1.0f;
    for (size_t i = 0; i < msg_->ranges.size(); ++i) {
      msg_->ranges[i] = distance / sin((msg_->angle_min + i * msg_->angle_increment));
    }

    rclcpp::Clock::SharedPtr clock = std::make_shared<rclcpp::Clock>(RCL_ROS_TIME);
    msg_->header.stamp = clock->now();

    pub_->publish(msg_);
  }

  rcl_lifecycle_transition_key_t
  on_configure(const rclcpp_lifecycle::State &)
  {
    msg_ = std::make_shared<sensor_msgs::msg::LaserScan>();
    pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>("scan");
    timer_ = this->create_wall_timer(
      10ms, std::bind(&ConfbotLaser::publish, this));

    msg_->header.frame_id = "laser_link";

    msg_->angle_increment = 5 * DEG2RAD;
    msg_->angle_min = 65 * DEG2RAD;
    msg_->angle_max = 115 * DEG2RAD;
    msg_->ranges.resize((msg_->angle_max - msg_->angle_min) / msg_->angle_increment);
    msg_->range_min = 0.0f;
    msg_->range_max = 10.0f;

    RCLCPP_DEBUG(get_logger(), "angle inc:\t%f", msg_->angle_increment)
    RCLCPP_DEBUG(get_logger(), "scan size:\t%zu", msg_->ranges.size())
    RCLCPP_DEBUG(get_logger(), "scan time increment: \t%f", msg_->time_increment)
    RCLCPP_INFO(get_logger(), "laser is configured")

    return lifecycle_msgs::msg::Transition::TRANSITION_CALLBACK_SUCCESS;
  }

  rcl_lifecycle_transition_key_t
  on_activate(const rclcpp_lifecycle::State &)
  {
    pub_->on_activate();

    RCUTILS_LOG_INFO_NAMED(get_name(), "laser is activated")

    return lifecycle_msgs::msg::Transition::TRANSITION_CALLBACK_SUCCESS;
  }

  rcl_lifecycle_transition_key_t
  on_deactivate(const rclcpp_lifecycle::State &)
  {
    pub_->on_deactivate();

    RCUTILS_LOG_INFO_NAMED(get_name(), "laser is deactivated")

    return lifecycle_msgs::msg::Transition::TRANSITION_CALLBACK_SUCCESS;
  }

  rcl_lifecycle_transition_key_t
  on_cleanup(const rclcpp_lifecycle::State &)
  {
    timer_.reset();
    pub_.reset();

    RCUTILS_LOG_INFO_NAMED(get_name(), "laser node is cleaning up")

    return lifecycle_msgs::msg::Transition::TRANSITION_CALLBACK_SUCCESS;
  }

private:
  std::shared_ptr<sensor_msgs::msg::LaserScan> msg_;

  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::LaserScan>> pub_;

  std::shared_ptr<rclcpp::TimerBase> timer_;
};

}  // namespace confbot_sensors

#endif  // CONFBOT_SENSORS__CONFBOT_LASER_HPP_
