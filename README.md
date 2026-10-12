# Robotics — Second Project

ROS 1 (Noetic) package `second_project` for an AgileX Scout Mini:

- **Task 1 — mapping:** build a map from the recorded bag (`/ugv/odom`, `/ugv/rslidar_points`)
  with slam_toolbox (`task_one.launch`) or gmapping (`gmapping_mapping.launch`).
- **Task 2 — navigation:** simulate the robot in Stage on that map, localize with AMCL and drive
  through a list of goals with move_base (`task_two.launch`).

## Setup

Requires ROS Noetic (EOL May 2025; Ubuntu 20.04 or the `ros:noetic` Docker image).

```bash
mkdir -p ~/catkin_ws/src && cd ~/catkin_ws/src
git clone https://github.com/lexdumitrache/robotics-second-project-final.git second_project
cd ~/catkin_ws && rosdep install --from-paths src --ignore-src -y
catkin_make && source devel/setup.bash
```

The course bag `robotics2.bag` is not in the repository (course rule: do not redistribute bags).
Put it in `second_project/bags/` (gitignored) or pass its path with `bag:=`.

## Run

```bash
# Task 1: mapping (slam_toolbox), RViz shows map, lidar and TF in the map frame
roslaunch second_project task_one.launch bag:=/path/to/robotics2.bag
rosrun map_server map_saver -f maps/slam_map        # save when the bag ends

# Task 1, alternative: gmapping
roslaunch second_project gmapping_mapping.launch bag:=/path/to/robotics2.bag

# Task 2: Stage + AMCL + move_base + goal publisher
roslaunch second_project task_two.launch                         # uses cfg/waypoints.csv
roslaunch second_project task_two.launch waypoints:=/path/to/goals.csv

# View a saved map
roslaunch second_project map_viewer.launch
```

### Data flow

```
Task 1   /ugv/odom ─ odom_to_tf ─▶ TF odom→base_link        static TF base_link→rslidar
         /ugv/rslidar_points ─ pointcloud_to_laserscan ─▶ /scan ─▶ slam_toolbox | gmapping ─▶ /map

Task 2   stageros ─▶ /base_scan, /odom, TF odom→base_footprint→base_link
         map_server(maps/slam_map.yaml) + amcl ─▶ TF map→odom
         goal_publisher ─(move_base action)─▶ move_base ─▶ /cmd_vel ─▶ stageros
```

### Goal file

One goal per line, `x,y,theta` in the `map` frame (metres, radians). Blank, header or malformed
lines are skipped with a warning. `goal_publisher` sends the next goal when the current one
succeeds or is aborted (assignment requirement), and also moves on after `~goal_timeout`
seconds (default 120). It exits with an error if move_base is not up within `~server_timeout`
(default 30 s). The bundled `cfg/waypoints.csv` points lie in free space reachable from the
start pose with ≥ 0.7 m clearance.

### Robot model

The Scout Mini is 0.612 × 0.580 m; costmaps use that rectangle as footprint. Stage simulates it as
a differential drive (`world/models/scout_mini.inc`), so the DWA planner has no lateral velocity
and AMCL uses the `diff-corrected` motion model.

### Maps

`maps/slam_map.*` is the slam_toolbox map used by Task 2 (and by Stage via `slam_map.png`).
`maps/gmapping_map.pgm` (~15 MB, 4000 × 4000 px default gmapping canvas) is kept only as the
gmapping result; `gmapping_map.png` is the compact copy.

## Tests

```bash
catkin_make run_tests && catkin_test_results   # waypoint CSV parser
```

## Troubleshooting

- *No map / TF errors in Task 1*: the bag must be played with `--clock` (the launch files do) and
  `/use_sim_time` must be true before nodes start.
- *Robot does not move in Task 2*: check `rostopic echo /move_base/status` and that AMCL has
  converged (set an initial pose in RViz if the particle cloud is spread).
