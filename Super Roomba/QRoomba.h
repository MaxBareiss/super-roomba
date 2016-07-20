#pragma once

#include "Roomba.h"
#include <tiny_cnn\tiny_cnn.h>

/// Q-Learning Applied to Roomba Control
class QRoomba : public Roomba {
public:
	RoombaCommand operator()(SensorState s);
	RoombaCommand commandFromAction(int action);
	RoombaCommand takeAction(SensorState ss);
	void learnFromResults(SensorState ss, long int reward);
	QRoomba();
	float epsilon = 1.0f, gamma = 0.01f;
private:
	void learn(SensorState state, tiny_cnn::vec_t Qs);
	tiny_cnn::vec_t generateInput(SensorState ss, bool hypothetical);
	tiny_cnn::vec_t getQs(SensorState s, bool hypothetical);
	tiny_cnn::network<tiny_cnn::sequential> net;
	deque < vector < float >> history;
	long int old_accum = 0;
	default_random_engine eng;
	tiny_cnn::vec_t Qs;
	int action;
	const unsigned int HISTORY = 10;
};