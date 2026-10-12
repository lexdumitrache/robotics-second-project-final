#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <second_project/waypoints.h>
#include <fstream>
#include <string>
#include <vector>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

std::vector<Waypoint> readGoalsFromCSV(const std::string &csv_file) {
    std::ifstream file(csv_file);
    std::vector<Waypoint> goals;
    if (!file) {
        ROS_ERROR("Cannot open waypoint file %s", csv_file.c_str());
        return goals;
    }
    std::string line;
    for (int n = 1; std::getline(file, line); n++) {
        Waypoint wp;
        if (parseWaypoint(line, wp)) {
            goals.push_back(wp);
        } else if (line.find_first_not_of(" \t\r") != std::string::npos) {
            ROS_WARN("%s:%d: skipping line, expected x,y,theta: '%s'", csv_file.c_str(), n, line.c_str());
        }
    }
    return goals;
}

void sendGoal(MoveBaseClient& ac, const Waypoint& goal) {
    move_base_msgs::MoveBaseGoal goal_msg;
    goal_msg.target_pose.header.frame_id = "map";
    goal_msg.target_pose.header.stamp = ros::Time::now();
    goal_msg.target_pose.pose.position.x = goal.x;
    goal_msg.target_pose.pose.position.y = goal.y;
    goal_msg.target_pose.pose.orientation.z = sin(goal.theta / 2.0);
    goal_msg.target_pose.pose.orientation.w = cos(goal.theta / 2.0);

    ac.sendGoal(goal_msg);
    ROS_INFO("Published waypoint at: %f %f %f", goal.x, goal.y, goal.theta);
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "goal_publisher");
    ros::NodeHandle nh("~");

    std::string csv_file;
    if (!nh.getParam("csv_file", csv_file)) {
        ROS_ERROR("Failed to get param 'csv_file'");
        return 1;
    }
    double server_timeout = nh.param("server_timeout", 30.0);  // s, 0 = wait forever
    double goal_timeout = nh.param("goal_timeout", 120.0);     // s per goal, 0 = no limit

    std::vector<Waypoint> goals = readGoalsFromCSV(csv_file);
    if (goals.empty()) {
        ROS_ERROR("No valid goals in %s", csv_file.c_str());
        return 1;
    }
    ROS_INFO("%zu goals have been loaded from %s", goals.size(), csv_file.c_str());

    MoveBaseClient ac("move_base", true);
    ROS_INFO("Waiting for the move_base action server...");
    if (!ac.waitForServer(ros::Duration(server_timeout))) {
        ROS_ERROR("move_base action server not available after %.0f s", server_timeout);
        return 1;
    }
    ROS_INFO("Connected to move_base server");

    // As required by the assignment, the next goal is sent once the current one is reached
    // or aborted. Rejected/preempted goals and timeouts are also skipped so the node never hangs.
    for (size_t i = 0; i < goals.size() && ros::ok(); i++) {
        sendGoal(ac, goals[i]);
        if (!ac.waitForResult(ros::Duration(goal_timeout))) {
            ac.cancelGoal();
            ROS_WARN("Goal %zu timed out after %.0f s, skipping", i, goal_timeout);
            continue;
        }
        actionlib::SimpleClientGoalState state = ac.getState();
        if (state == actionlib::SimpleClientGoalState::SUCCEEDED) {
            ROS_INFO("Goal %zu reached", i);
        } else {
            ROS_WARN("Goal %zu finished with state %s, moving to the next one", i, state.toString().c_str());
        }
    }

    ROS_INFO("All goals have been processed.");
    return 0;
}
