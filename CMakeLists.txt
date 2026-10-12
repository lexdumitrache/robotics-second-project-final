cmake_minimum_required(VERSION 3.0.2)
project(second_project)

find_package(catkin REQUIRED COMPONENTS
  actionlib
  move_base_msgs
  nav_msgs
  roscpp
  tf
)
catkin_package()
include_directories(include ${catkin_INCLUDE_DIRS})

add_executable(odom_to_tf src/odom_to_tf.cpp)
target_link_libraries(odom_to_tf ${catkin_LIBRARIES})

add_executable(goal_publisher src/goal_publisher.cpp)
target_link_libraries(goal_publisher ${catkin_LIBRARIES})

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(test_waypoints test/test_waypoints.cpp)
endif()
