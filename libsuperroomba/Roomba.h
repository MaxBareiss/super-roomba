#pragma once
#include "Types.h"

class Roomba {
public:
	Vec loc;
	float theta;
	const float r = 0.17f;
	virtual RoombaCommand operator()(SensorState s)=0;
};