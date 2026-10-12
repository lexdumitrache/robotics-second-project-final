#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

std::vector<std::tuple<double, double, double>> readGoalsFromCSV(const std::string &csv_file) {
    std::ifstream file(csv_file);
    std::string line;
    std::vector<std::tuple<double, double, double>> goals;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        double x, y, theta;

        std::getline(ss, token, ',');
        x = std::stod(token);
        std::getline(ss, token, ',');
        y = std::stod(token);
        std::getline(ss, token, ',');
        theta = std::stod(token);

        goals.emplace_back(x, y, theta);
    }

    return goals;
}

void sendGoal(MoveBaseClient& ac, const std::tuple<double, double, double>& goal) {
    move_base_msgs::MoveBaseGoal goal_msg;
    double x = std::get<0>(goal);
    double y = std::get<1>(goal);
    double theta = std::get<2>(goal);
    goal_msg.target_pose.header.frame_id = "map";
    goal_msg.target_pose.header.stamp = ros::Time::now();
    goal_msg.target_pose.pose.position.x = x;
    goal_msg.target_pose.pose.position.y = y;
    goal_msg.target_pose.pose.orientation.z = sin(theta / 2.0);
    goal_msg.target_pose.pose.orientation.w = cos(theta / 2.0);

    ac.sendGoal(goal_msg);
    ROS_INFO("Published waypoint at: %f %f %f", x, y, theta);
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "goal_publisher");
    ros::NodeHandle nh("~");

    std::string csv_file;
    if (!nh.getParam("csv_file", csv_file)) {
        ROS_ERROR("Failed to get param 'csv_file'");
        return 1;
    }

    std::vector<std::tuple<double, double, double>> goals = readGoalsFromCSV(csv_file);
    ROS_INFO("%li goals have been loaded from %s", goals.size(), csv_file.c_str());

    MoveBaseClient ac("move_base", true);
    ROS_INFO("Waiting for the move_base action server...");
    ac.waitForServer();
    ROS_INFO("Connected to move_base server");

    int current_goal = 0;
    while (ros::ok() && current_goal < goals.size()) {
        sendGoal(ac, goals[current_goal]);

        bool goal_reached = false;
        while (!goal_reached && ros::ok()) {
            actionlib::SimpleClientGoalState state = ac.getState();
            if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
                ROS_INFO("Goal %d reached", current_goal);
                goal_reached = true;
            } else if (state == actionlib::SimpleClientGoalState::ABORTED) {
                ROS_WARN("Goal %d aborted, retrying...", current_goal);
                goal_reached = true; // Proceed to the next goal even if this one is aborted
            }
            ros::Duration(0.5).sleep();
            ros::spinOnce();
        }

        current_goal++;
    }

    ROS_INFO("All goals have been processed.");
    return 0;
}
