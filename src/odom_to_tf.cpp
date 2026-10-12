#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include <nav_msgs/Odometry.h>

void odometryCallback(const nav_msgs::Odometry::ConstPtr& msg) {
    static tf::TransformBroadcaster br;
    tf::Transform transform;
    transform.setOrigin(tf::Vector3(msg->pose.pose.position.x, 
                                    msg->pose.pose.position.y, 
                                    msg->pose.pose.position.z));
    transform.setRotation(tf::Quaternion(msg->pose.pose.orientation.x, 
                                         msg->pose.pose.orientation.y, 
                                         msg->pose.pose.orientation.z, 
                                         msg->pose.pose.orientation.w));
    br.sendTransform(tf::StampedTransform(transform, msg->header.stamp, "odom", "base_link"));
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "odom_to_tf");
    ros::NodeHandle n;

    ros::Subscriber sub = n.subscribe<nav_msgs::Odometry>("ugv/odom", 100, 
        std::bind(odometryCallback, std::placeholders::_1));
    ROS_INFO("Odom to tf converter started!");
    ros::spin();
    return 0;
}