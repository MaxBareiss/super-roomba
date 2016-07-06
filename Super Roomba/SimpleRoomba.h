#pragma once
#include "Types.h"
#include "Roomba.h"

class SimpleRoomba : public Roomba {
public:
	enum State {
		GOING_FORWARD,
		BACKING_UP,
		ROTATING_AWAY,
		LONG_BACKUP
	} state;
private:
	int count;
	bool rotLeft;
	RoombaCommand operator()(SensorState s);
};