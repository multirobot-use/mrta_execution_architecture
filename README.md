# Mission Execution Architecture for Multi-UAV Teams

## Overview

This repository contains an architecture for mission planning and execution in heterogeneous teams of UAVs. The system addresses plan monitoring and execution in the context of Multi-Robot Task Allocation missions and it is implemented in [ROS](https://ros.org/) (Robot Operating System)(MRTA).

This software architecture consists of two layers: a High-Level Planner, centralized on the ground station, and an Agent Behavior Manager, distributed on board each UAV. In this way, the High-Level Planner receives task requests as input, and its work is to solve a resource-constrained problem that allows distributing tasks among the team taking into account vehicles’ capabilities and battery constraints. The Agent Behavior Manager, based on Behavior Trees, is in charge of executing and supervising those plans, calling the appropriate lower-level controllers at any given time. The controllers for each specific action inside each of the tasks are not included in this work, but simple versions of those controllers can be found in this repository in order to be able to test the software layer properly in simulation. Last, replanning operations are triggered in case of unforeseen events, such as vehicle faults or communication drop-outs.

The system is flexible and different modules could be plugged in as High-level Planner as long as they resolve MRTA missions for heterogeneous teams of UAVs. A specific MRTA planner in a separate [repository](https://github.com/multirobot-use/mrta_heuristic_planner), written in Matlab, has been successfully integrated in the architectured contained in the current repository. This Matlab code is connected with ROS through the [matlab_ros_connector](scripts/matlab_ros_connector.m) script available in the scripts folder, which implements a ROS Action Server that receives planning and replanning requests from a High-Level Planner.

If you are using this software layer or you found this approach inspiring for your own research, please cite:

```bibtex
@ARTICLE{calvo_tro25,
  author={Calvo, Alvaro and Capitan, Jesus},
  journal={IEEE Transactions on Robotics}, 
  title={Heterogeneous Multirobot Task Allocation for Long-Endurance Missions in Dynamic Scenarios}, 
  year={2025},
  volume={41},
  number={},
  pages={6494-6513},
  doi={10.1109/TRO.2025.3626651}
  }
@INPROCEEDINGS{CalvoICUAS22,  
  author        = {Calvo, Alvaro and Silano, Giuseppe and Capitan, Jesus},  
  booktitle     = {2022 International Conference on Unmanned Aircraft Systems (ICUAS)},   
  title         = {Mission Planning and Execution in Heterogeneous Teams of Aerial Robots supporting Power Line Inspection Operations},
  year          = {2022},  
  pages         = {1644-1649},
  doi           = {10.1109/ICUAS54217.2022.9836234}
}
```

## Installation

This software has been developed on Ubuntu 20.04 with ROS Noetic. To install the repository correctly, you have to follow the next steps:

0.1. ROS Noetic installation

```bash
sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
sudo apt update
sudo apt install -y ros-noetic-desktop-full
echo "source /opt/ros/noetic/setup.bash" >> ~/.bashrc
source ~/.bashrc
sudo apt install -y python-rosdep python-rosinstall python-rosinstall-generator python-wstool build-essential
sudo rosdep init
rosdep update
```

0.2. (Recomended) Catkin tools

```bash
sudo sh \
    -c 'echo "deb http://packages.ros.org/ros/ubuntu `lsb_release -sc` main" \
        > /etc/apt/sources.list.d/ros-latest.list'
wget http://packages.ros.org/ros.key -O - | sudo apt-key add -
sudo apt-get update
sudo apt-get install -y python-catkin-tools
```

1. Install necessary packages

```bash
sudo apt install -y libeigen3-dev ros-noetic-geodesy ros-noetic-joy ros-noetic-multimaster-fkie
sudo pip install pynput
sudo apt install -y xz-utils
sudo apt-get install -y libzmq3-dev libboost-dev
sudo apt-get install -y ros-noetic-behaviortree-cpp-v3
```

2. Create a ROS workspace

```bash
mkdir -p ~/mission_planner_ws/src
cd ~/mission_planner_ws/
catkin build -DPYTHON_EXECUTABLE=/usr/bin/python
source devel/setup.bash
echo "source $HOME/mission_planner_ws/devel/setup.bash" >> ~/.bashrc
```

3. Clone this repository and other ones as dependencies

```bash
cd ~/mission_planner_ws/src/
git clone https://github.com/multirobot-use/mrta_execution_architecture.git
git clone https://github.com/multirobot-use/mrta_heuristic_planner.git
git clone https://github.com/grvcTeam/grvc-ual.git
git clone https://github.com/grvcTeam/grvc-utils.git
```

5. Ignore some packages

```bash
touch ~/mission_planner_ws/src/grvc-utils/mission_lib/CATKIN_IGNORE
```

6. Install Groot and its dependencies

```bash
cd ~/mission_planner_ws/src/
sudo apt install -y qtbase5-dev libqt5svg5-dev libzmq3-dev libdw-dev
git clone https://github.com/BehaviorTree/Groot.git
touch ~/mission_planner_ws/src/Groot/CATKIN_IGNORE
cd ..
rosdep install --from-paths src --ignore-src
catkin build
```

7. Install and configure UAL. Only MAVROS needed. Make sure to install its dependencies when asked

```bash
cd ~/mission_planner_ws/src/grvc-ual
./configure.py
```

8. Install MAVROS packages

```bash
sudo apt install -y ros-noetic-mavros ros-noetic-mavros-extras
sudo geographiclib-get-geoids egm96-5
sudo usermod -a -G dialout $USER
sudo apt remove modemmanager
```

9.1 (Optional) Install RealSense plugins for real-life execution

```bash
sudo apt install -y ros-noetic-realsense2-camera ros-noetic-realsense2-description
```

9.2 (Optional) Download 99-realsense-libusb.rules file from [github](https://github.com/IntelRealSense/librealsense/blob/master/config/99-realsense-libusb.rules)

9.3 (Optional) Give permissions to read the data from the RealSense camera

```bash
sudo cp 99-realsense-libusb.rules /etc/udev/rules.d/99-realsense-libusb.rules
```

10. Install PX4 for SITL simulations

```bash
sudo apt install -y libgstreamer1.0-dev python-jinja2 python-pip
pip install numpy toml
cd ~/mission_planner_ws/src/
git clone https://github.com/PX4/Firmware.git
cd Firmware
git checkout v1.10.2
git submodule update --init --recursive
make
make px4_sitl_default gazebo
```

11. Build

```bash
cd ~/mission_planner_ws/
catkin build
```

**Note**: In case that the installation did not go well, try compile each package individually.

12. Matlab setup

To use ROS with Matlab, we need the [ROS Toolbox](https://www.mathworks.com/products/ros.html). Once the toolbox is installed, in order to set up the Matlab-ROS connection, you need to open Matlab as admin (to be able to save Matlab's path) and then run the [gen_matlab_msgs](scripts/gen_matlab_msgs.m) script.

```bash
roscd mission_planner
cd scripts
sudo matlab -nodisplay -nosplash -r "gen_matlab_msgs; exit"
```

You will also need to include the [mrta_heuristic_planner](https://github.com/multirobot-use/mrta_heuristic_planner) installation folder and subfolders in the MATLAB's path.

If you experience problems related with the python executable while executing the above script, you may find useful to create a virtual python environment. For instance, if ROS needs python3.8 to be the default python executable, but MATLAB 2023b needs python3.9., creating a virtual python environment and specifying it's executable route in the ROS Toolbox Preferences could solve this issue:

```bash
cd ~
python3.9 -m venv matlab_env
```

```matlab
pyenv('Version', '~/matlab_env/bin/python3.9')
```

## Test

To test if the system is working correctly, you can launch a simulation and order tasks or unexpected events by executing Makefile recipes.

**Note**: The `Simulation.launch` file has some parameters to facilitate the configuration of the simulations, having parameters to select the number of UAVs, the Gazebo world, debug modes and some other things. See the heading of the file to know the different world options.

```bash
make launch
...
make monitor task_id=1 human_target=human_target_1 number=2 distance=1.5
make inspect task_id=2
make deliver task_id=3 human_target=human_target_1 tool=hammer
...
make battery_off agent_id=1
make battery_ok  agent_id=1
...
make mission_over
...
rosnode kill /uav_1/agent_behaviour_manager
rosrun mission_planner agent_behaviour_manager __ns:uav_1
...
rosnode kill /high_level_planner
```

**Note**: For information on how to launch tasks manually, you can run `make gesture_info` or just read the recipes in the Makefile.

You can also use the `tmuxinator` tool to launch the simulation nodes in a more organized way using the [launch script](scripts/launch_matlab_ros_connection.sh) available in the scripts folder. This script creates a tmux session according to the specifications present in the [config file](scripts/matlab_ros_connection.yml). To use this method, you'll need to install `tmuxinator` and incorporate some shell additions to our terminal:

```bash
sudo apt install tmuxinator
chmod +x scripts/launch_matlab_ros_connection.sh
source scripts/shell_additions.sh
```

Now you can launch the simulation using `tmuxinator`:

```bash
./scripts/launch_matlab_ros_connection.sh
```

## Monitoring the Behavior Tree execution with Groot

There is a Make recipe to launch a Groot node that monitors the execution of the Behavior Trees in real time or replays a log file.


```bash
make groot
```

To monitor a Behavior Tree, you just has to specify:

* Server IP: localhost
* Publisher Port: 1666 + (ID - 1) * 2
* Server Port: 1667 + (ID - 1) * 2

E.g., for the UAV_1:

* Server IP: localhost
* Publisher Port: 1666
* Server Port: 1667

**Note**: fbl log files are stored in `~/.ros` and are named as `bt_trace_uav_` + ID + `.fbl`
