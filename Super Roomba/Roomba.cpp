#include "Roomba.h"

RoombaCommand Roomba::operator()(SensorState s)
{
	RoombaCommand cmd;
	switch (state)
	{
	case Roomba::GOING_FORWARD:
		if (s.center || s.left || s.right) {
			state = BACKING_UP;
			count = 0;
		}
		break;
	case Roomba::BACKING_UP:
		if (count > 50) {
			state = ROTATING_AWAY;
			rotLeft = s.wall_left < s.wall_right;
			count = 0;
		}
		break;
	case Roomba::ROTATING_AWAY:
		if (count > 50) {
			state = GOING_FORWARD;
		}
		break;
	default:
		state = GOING_FORWARD;
		break;
	}
	switch (state)
	{
	case Roomba::GOING_FORWARD:
		if (s.wall_left > 0.3 && s.wall_right > 0.3) {
			cmd = { 0.3f, 0.3f };
		} else {
			cmd = { 0.1f, 0.1f };
		}
		break;
	case Roomba::BACKING_UP:
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
		state = GOING_FORWARD;
		break;
	}
	++count;
	return cmd;
}