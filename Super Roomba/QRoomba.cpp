#include "QRoomba.h"
#include <random>

RoombaCommand QRoomba::operator()(SensorState s)
{
	RoombaCommand c = { 0,0 };
	return c;
}

vector<float> packageData(SensorState s) {
	vector<float> out;
	out.resize(8);
	out[0] = s.left ? 1 : 0;
	out[1] = s.left ? 0 : 1;
	out[2] = s.right ? 1 : 0;
	out[3] = s.right ? 0 : 1;
	out[4] = s.center ? 1 : 0;
	out[5] = s.center ? 0 : 1;
	out[6] = s.wall_left;
	out[7] = s.wall_right;
	return out;
}

tiny_cnn::vec_t QRoomba::getQs(SensorState s) {
	tiny_cnn::vec_t res = net.predict(packageData(s));
	return res;
}

RoombaCommand QRoomba::commandFromAction(int action) {
	switch (action) {
	case 0:
		return{ 0.4f, 0.4f };
	case 1:
		return{ -0.4f, -0.4f };
	case 2:
		return{ -0.4f, 0.4f };
	case 3:
		return{ 0.4f,-0.4f };
	}
}

int argmax(tiny_cnn::vec_t vec) {
	int i = 0;
	float winnerIdx = 0;
	float winner = -INFINITY;
	for (float elm : vec) {
		if (elm > winner) {
			winner = elm;
			winnerIdx = i;
		}
		++i;
	}
	return winnerIdx;
}

void QRoomba::learn(SensorState state, tiny_cnn::vec_t Qs) {
	using namespace tiny_cnn;
	using namespace tiny_cnn::activation;
	vec_t vecState;
	vector<float> packed = packageData(state);
	vecState.resize(packed.size());
	for (int i = 0; i < packed.size(); ++i) {
		vecState[i] = packed[i];
	}
	net.fit<mse>(adagrad(), vector<vec_t>{ vecState }, vector<vec_t>{ Qs }, 1, 1);
}

RoombaCommand QRoomba::takeAction(SensorState ss) {
	Qs = getQs(ss);
	action = 0;
	if (uniform_real_distribution<float>(0, 1)(eng) < epsilon) {
		action = uniform_int_distribution<int>(0, 3)(eng);
	}
	else {
		action = argmax(Qs);
	}
	return commandFromAction(action);
}

void QRoomba::learnFromResults(SensorState ss, long int reward) {
	int maxQ = argmax(getQs(ss));
	Qs[action] = reward + (gamma*maxQ);
	learn(ss, Qs);
}

QRoomba::QRoomba() {
	using namespace tiny_cnn;
	using namespace tiny_cnn::activation;

	net = make_mlp<tan_h>({ 8,20,100,20,4 });
}