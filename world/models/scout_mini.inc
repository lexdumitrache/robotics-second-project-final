define kinect ranger
(
  sensor
  (
    range_max 8.0
    fov 120.0
    samples 640
  )
  # generic model properties
  color "black"
  size [ 0.06 0.15 0.03 ]
)

define scout_mini position
(
  pose [ 0.0 0.0 0.0 0.0 ]

  odom_error [0.01 0.01 999999 999999 999999 0.01]

  size [ 0.612 0.580 0.245 ]
  origin [ 0.0 0.0 0.0 0.0 ]
  gui_nose 1
  drive "diff"
  color "gray"
  
  kinect(pose [ -0.1 0.0 -0.11 0.0 ])
)
