#include "Roomba.h"
#include <random>
#include <iostream>

using namespace std;

RoombaCommand Roomba::operator()(SensorState s)
{
	RoombaCommand cmd;
	auto oldState = state;
	switch (state)
	{
	case Roomba::GOING_FORWARD:
		if (s.center || s.left || s.right) {
			if (count > 100) {
				state = BACKING_UP;
				//cout << count << endl;
				count = 0;
			}else{
				//cout << count << endl;
				state = LONG_BACKUP;
				count = 0;
			}
		}
		break;
	case Roomba::LONG_BACKUP:
		//cout << "LONG_BACKUP! " << count << endl;
		if (count > 400 + uniform_int_distribution<int>(1, 100)(default_random_engine())) {
			//cout << count << endl;
			state = ROTATING_AWAY;
			count = 0;
			rotLeft = true;
		}
		break;
	case Roomba::BACKING_UP:
		if (count > 50 + uniform_int_distribution<int>(1,100)(default_random_engine())) {
			state = ROTATING_AWAY;
			//rotLeft = s.wall_left < s.wall_right;
			rotLeft = true;
			count = 0;
		}
		break;
	case Roomba::ROTATING_AWAY:
		if (count > 50 + uniform_int_distribution<int>(1, 20)(default_random_engine())) {
			state = GOING_FORWARD;
			count = 0;
		}
		break;
	default:
		state = GOING_FORWARD;
		count = 0;
		break;
	}
	if (oldState != state) {
		//cout << "OLDSTATE: " << int(oldState) << " NEW STATE: " << state << endl;
	}
	switch (state)
	{
	case Roomba::GOING_FORWARD:
		if (s.wall_left > 0.25 && s.wall_right > 0.25) {
			cmd = { 0.3f, 0.3f };
		} else {
			cmd = { 0.1f, 0.1f };
		}
		break;
	case Roomba::BACKING_UP:
		cmd = { -0.1f,-0.1f };
		break;
	case Roomba::LONG_BACKUP:
		cmd = { -0.1f,-0.1f };
		break;
	case Roomba::ROTATING_AWAY:
		if (rotLeft) {
			cmd = { 0.1f,-0.1f };
		} else {
			cmd = { -0.1f,0.1f };
		}
		break;
	default:
		cmd = { 0.0f,0.0f };
		break;
	}
	++count;
	return cmd;
}