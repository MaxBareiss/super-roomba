#pragma once

#include <vector>

using namespace std;

struct Vec {
	float x, y;
	Vec operator+ (Vec rhs) {
		return{ x + rhs.x, y + rhs.y };
	}
	Vec operator- (Vec rhs) {
		return{ x - rhs.x, y - rhs.y };
	}
	float dot(Vec rhs) {
		return x*rhs.x + y*rhs.y;
	}
	Vec operator* (float rhs) {
		return{ x*rhs,y*rhs };
	}
	Vec operator/ (float rhs) {
		return{ x/rhs,y/rhs };
	}
};

struct Obstacle {
	Vec loc;
	float r;
};

struct Segment {
	Vec P0;
	Vec P1;
};

struct Room {
	vector<Vec> corners;
	vector<Obstacle> obstacles;
};

struct SensorState {
	bool left, center, right;
	float wall_left, wall_right;
};

struct RoombaCommand {
	float v_l, v_r;
};

struct Ray {
	Vec point;
	float theta;
};