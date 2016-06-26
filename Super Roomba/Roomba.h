#pragma once
#include "Types.h"

class Roomba {
public:
	Vec loc;
	float theta;
	const float r = 0.17f;
	enum State {
		GOING_FORWARD,
		BACKING_UP,
		ROTATING_AWAY
	} state;
	RoombaCommand operator()(SensorState s);
private:
	int count;
	bool rotLeft;
};