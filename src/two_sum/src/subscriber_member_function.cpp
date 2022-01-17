// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int8.hpp"
#include "std_msgs/msg/int8_multi_array.hpp"


using std::placeholders::_1;
using namespace std::chrono_literals;

class SubscriberAndPublisher : public rclcpp::Node
{
public:
  SubscriberAndPublisher()
  : Node("SubscriberAndPublisher")
  {
    set = false;
    subscription_input = this->create_subscription<std_msgs::msg::Int8MultiArray>(
      "/input", 10, std::bind(&SubscriberAndPublisher::input_callback, this, _1));

    subscription_target = this->create_subscription<std_msgs::msg::Int8>(
      "/target", 10, std::bind(&SubscriberAndPublisher::target_callback, this, _1));

    publisher_ = this->create_publisher<std_msgs::msg::Int8MultiArray>("/solution", 10); //Send array result every second
    timer_ = this->create_wall_timer(
      1000ms, std::bind(&SubscriberAndPublisher::timer_callback, this));
  }

private:

  void input_callback(const std_msgs::msg::Int8MultiArray::ConstSharedPtr msg) //Saving input
  {
    input.data.clear();
    input.data.insert(input.data.begin(), msg->data.begin(), msg->data.end());

  }

  void target_callback(const std_msgs::msg::Int8::ConstSharedPtr msg) //Saving target
  {
    target.data = msg->data;
    set = true;
  }

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::Int8MultiArray>::SharedPtr publisher_;

  std_msgs::msg::Int8 target;
  std_msgs::msg::Int8MultiArray result;
  std_msgs::msg::Int8MultiArray input;
  bool set;
  rclcpp::Subscription<std_msgs::msg::Int8MultiArray>::SharedPtr subscription_input;
  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr subscription_target;

  void timer_callback()
  {
    if(input.data.empty() || !set) //If no data from subscribers
      return;

    calc_result();

    std_msgs::msg::Int8MultiArray message;

    message.data.clear();
    message.data.insert(message.data.end(), result.data.begin(), result.data.end());
    publisher_->publish(message);
  }

  void calc_result() //Find indices that add upto target
  {
    result.data.clear();
    std::unordered_map<int,int> map;

    for(int i=0; i< (int)input.data.size(); i++){
      int8_t temp = target.data - input.data[i];

      if(map.find(temp) != map.end()){
          result.data.push_back(i);
          result.data.push_back(map[temp]);
          return;
      }
      map[input.data[i]] = i;
    }
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SubscriberAndPublisher>());
  rclcpp::shutdown();
  return 0;

}
