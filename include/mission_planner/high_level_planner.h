#ifndef TASK_PLANNER_H
#define TASK_PLANNER_H

#include "ros/ros.h"
#include <ros/package.h>
#include <thread>
#include "yaml-cpp/yaml.h"

#include <string>
#include <queue>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <algorithm>
#include <actionlib/server/simple_action_server.h>

#include "mission_planner/classes.h"
#include "mission_planner/AgentBeacon.h"
#include "mission_planner/PlannerBeacon.h"
#include "mission_planner/NewTaskAction.h"
#include "mission_planner/NewTaskListAction.h"
#include "mission_planner/TaskResultAction.h"
#include "mission_planner/BatteryEnoughAction.h"
#include "mission_planner/MissionOver.h"
#include "mission_planner/Waypoint.h"
#include "mission_planner/Task.h"
#include "mission_planner/DoCloserInspectionAction.h"
#include "mission_planner/HeuristicPlanningAction.h"

#include "geometry_msgs/PoseStamped.h"
#include "sensor_msgs/BatteryState.h"

//Forward declarations
class Planner;

class Agent{
  private:
    std::string id_;
    std::string type_;
    std::queue<classes::Task*> task_queue_;
    std::queue<classes::Task*> old_task_queue_;
	std::string old_first_task_id_;

	Planner* planner_;
	ros::Time last_beacon_time_;
	mission_planner::AgentBeacon last_beacon_;

	//Node Handlers
	ros::NodeHandle nh_;

    //Subscribers
	ros::Subscriber position_sub_;
	ros::Subscriber battery_sub_;
	std::string pose_topic_;
	std::string battery_topic_;
	classes::Position position_;
    float battery_; //percentage

    //Actions
	actionlib::SimpleActionClient<mission_planner::NewTaskListAction> ntl_ac_;
	actionlib::SimpleActionServer<mission_planner::BatteryEnoughAction> battery_as_;
    actionlib::SimpleActionServer<mission_planner::TaskResultAction> task_result_as_;
	bool battery_enough_;
	mission_planner::BatteryEnoughFeedback battery_feedback_;
	mission_planner::BatteryEnoughResult battery_result_;

  public:
    //Constructors
    Agent();
		Agent(Planner* planner, std::string id, std::string type, ros::Time first_beacon_time,
				mission_planner::AgentBeacon first_beacon);
    Agent(const Agent& a);
    ~Agent();

    //Topic methods
	void updateSensorsInformation();
	bool isBatteryForQueue();
	bool isBatteryEnough(classes::Task* task);
	bool checkBeaconTimeout(ros::Time now);

    //Task queue methods
    bool isQueueEmpty();
    void emptyTheQueue();
    void addTaskToQueue(classes::Task* task);
    void replaceTaskFromQueue(std::string task_id);
	classes::Task* getFirstTask();
	classes::Task* getOldFirstTask();
	classes::Task* getLastTask();
	bool isTaskInQueue(std::string task_id);
	void setOldTaskQueue();
	void deleteOldTaskQueue();
    int getQueueSize();
	void sendQueueToAgent();
	float computeTaskCost(classes::Task* task);

	//Getters
    std::string getID();
    std::string getType();
	float getBattery();
	bool getLastBeaconTimeout();

	//Setters
	void setLastBeaconTime(ros::Time last_beacon_time);
	void setLastBeacon(mission_planner::AgentBeacon last_beacon);

	//Callbacks
    void positionCallbackUAL(const geometry_msgs::PoseStamped& pose);
    void batteryCallback(const sensor_msgs::BatteryState& battery);
	void batteryEnoughCB(const mission_planner::BatteryEnoughGoalConstPtr& goal);
	void taskResultCB(const mission_planner::TaskResultGoalConstPtr& goal);

    //Visualization method
    void print(std::ostream& os);
};

std::ostream& operator << (std::ostream& os, Agent& a){
  a.print(os);
  return os;
}

class Planner {
  private:
    //Node Handlers
    ros::NodeHandle nh_;

	std::unique_ptr<actionlib::SimpleActionClient<mission_planner::HeuristicPlanningAction>> hp_ac_;
	// actionlib::SimpleActionClient<mission_planner::HeuristicPlanningAction> hp_ac_;
	actionlib::SimpleActionServer<mission_planner::NewTaskAction> nt_as_;

    mission_planner::NewTaskFeedback nt_feedback_;
    mission_planner::NewTaskResult nt_result_;

    // Subscribers
    ros::Subscriber beacon_sub_;
	ros::Subscriber mission_over_sub_;
	bool mission_over_;

    // Publishers
	ros::Publisher beacon_pub_;
    mission_planner::PlannerBeacon beacon_;
    ros::Rate beacon_rate_;

    std::string config_file;
	std::string mission_id_;

	std::map <std::string, std::map <std::string, classes::Position>> known_positions_;
    std::map <std::string, classes::HumanTarget> human_targets_;
    std::map <std::string, classes::Tool> tools_;

    std::map <std::string, Agent> agent_map_;
	std::vector <std::string> deliver_agents_;
	std::vector <std::string> inspect_agents_;
	std::vector <std::string> monitor_agents_;

    std::map <std::string, classes::Task*> pending_tasks_;
	classes::Task* recharge_task_;
	std::vector <std::string> deliver_tasks_;
	std::vector <std::string> inspect_tasks_;
	std::vector <std::string> monitor_tasks_;

  protected:
    void readConfigFile(std::string config_file);

  public:
	Planner(mission_planner::PlannerBeacon beacon);
    ~Planner(void);

	bool checkTaskParams(const mission_planner::NewTaskGoalConstPtr& goal);

    //New Tasks callback
    void incomingTask(const mission_planner::NewTaskGoalConstPtr& goal);

    //Agent Beacon Handler
    void beaconCallback(const mission_planner::AgentBeacon::ConstPtr& beacon);
	void missionOverCallback(const mission_planner::MissionOver& value);

    //Method to reassign all not finished tasks
    void performTaskAllocation();

	//Pending Tasks methods
	classes::Task* getPendingTask(std::string task_id);
	void deletePendingTask(std::string task_id);
	bool updateTaskParams(const mission_planner::NewTaskGoalConstPtr& goal);
	void checkBeaconsTimeout(ros::Time now);
	
	//Getters
	bool getMissionOver();

	// Others
	bool isTopicAvailable(const std::string &topic_name);
};

struct Cost{
	public:
		Cost(float cost, std::string id) : cost_(cost), id_(id) {}

		bool operator < (const Cost& cost) const {return (cost_ < cost.cost_);}
		bool operator > (const Cost& cost) const {return (cost_ > cost.cost_);}
		bool operator == (const Cost& cost) const {return (cost_ == cost.cost_);}

		float cost_;
		std::string id_;
};

#endif
