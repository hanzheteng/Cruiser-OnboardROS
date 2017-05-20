/*
 * tracking_move.cpp
 *
 *  Created on: May 10, 2017
 *      Author: cj
 */
#include <ros/ros.h>
#include <cruiser/CruiserHeader.h>
#include <cruiser/CruiserDrone.h>
#include <dji_sdk/dji_drone.h>

class TrackingMove
{
private:

	ros::Subscriber ChangeFlag;
	ros::Subscriber DeltaMsg;
public:

	DJIDrone *drone;
	bool delta_pos = false;
	float delta_x_pos = 0;
	float delta_y_pos = 0;

	TrackingMove(ros::NodeHandle& nh)
	{
		drone = new DJIDrone(nh);
		DeltaMsg = nh.subscribe("cruiser/tracking_move",1,&TrackingMove::DeltaMsgCallback,this);
		ChangeFlag = nh.subscribe("cruiser/tracking_flag",1,&TrackingMove::ChangeGimbalAngle,this);
	}


	void ChangeGimbalAngle(const cruiser::Flag &tracking_flag)
	{
		if(tracking_flag.flag)
		{
			this->drone->gimbal_angle_control(0, -450, 0, 10);
			usleep(10000);
		}
	}

	void DeltaMsgCallback(const cruiser::DeltaPosition& new_location)
	{
		this->delta_pos = new_location.state;
		if (new_location.state)
		{
			this->delta_x_pos = new_location.delta_X_meter;
			this->delta_y_pos = new_location.delta_Y_meter;
		}
	}

};

int main(int argc,char **argv)
{
	ros::init(argc,argv,"tracking_move_node");
	ros::NodeHandle nh;
	TrackingMove *landing_move_node = new TrackingMove(nh);
    ros::Rate rate(1);

	if(landing_move_node->drone->request_sdk_permission_control())
		ROS_INFO_STREAM("tracking_move_node : initialization and get control.");

    while(ros::ok())
    {
    	if(landing_move_node->delta_pos)
    	{
        	landing_move_node->drone->attitude_control(0x82,landing_move_node->delta_x_pos,landing_move_node->delta_y_pos,0,0);//location
        	usleep(500000);
    	}
        ros::spinOnce();
        rate.sleep();
    }
}

