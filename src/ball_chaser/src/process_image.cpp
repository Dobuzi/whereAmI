#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

ros::ServiceClient client;

void drive_robot(float lin_x, float ang_z) {
  ROS_INFO_STREAM("Moving...");

  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;

  if (!client.call(srv)) {
    ROS_ERROR("Failed to call service drive robot");
  }
}

void process_image_callback(const sensor_msgs::Image img) {
  int white_pixel = 255;
  int left = 0;
  int forward = 0;
  int right = 0;
  float lin_x = 0.0;
  float ang_z = 0.0;
  int max_zone = 0;
  int max_threshold = 100000;
  int min_threshold = 100;
  float turn_left = -0.3;
  float turn_right = 0.3;
  float run_forward = 0.3;

  for (int i = 0; i < img.height * img.step; i++) {
    if (img.data[i] == white_pixel) {
      int position = i % img.step;
      if (position < (img.step / 3)) {
        left++;
      } else if (position < (img.step * 2 / 3)) {
        forward++;
      } else {
        right++;
      }
    }
  }

  if (left > forward) {
    if (left >  right) {
      max_zone = left;
      ang_z = turn_left;
    } else {
      max_zone = right;
      ang_z = turn_right;
    }
  } else {
    if (forward > right) {
      max_zone = forward;
      lin_x = run_forward;
    } else {
      max_zone = right;
      ang_z = turn_right;
    }
  }

  ROS_INFO_STREAM("left: " + std::to_string(left) + ", forward:" + std::to_string(forward) + ", right: " + std::to_string(right));
  
  if (max_zone > min_threshold) {
    if (max_zone > max_threshold) {
      lin_x = 0.0;
      ang_z = 0.0;
    }
    drive_robot(lin_x, ang_z);
  }
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "process_image");
  ros::NodeHandle n;

  client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

  ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

  ros::spin();

  return 0;
}