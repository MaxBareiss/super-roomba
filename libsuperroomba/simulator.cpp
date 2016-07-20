//#include "stdafx.h"
#include "simulator.h"
#include <iostream>

using namespace std;
// http://geomalgorithms.com/a03-_inclusion.html

// isLeft(): tests if a point is Left|On|Right of an infinite line.
//    Input:  three points L1, L2, and P
//    Return: >0 for P left of the line through L1 and L2
//            =0 for P on the line
//            <0 for P right of the line
//    See: Algorithm 1 "Area of Triangles and Polygons"
inline int isLeft(Vec L1, Vec L2, Vec P) {
	return ((L2.x - L1.x) * (P.y - L1.y)
		- (P.x - L1.x) * (L2.y - L1.y));
}

Vec operator* (float lhs, Vec rhs) {
	return{ rhs.x*lhs,rhs.y*lhs };
}

int is_inside(Vec P, vector<Vec> V)
{
	int wn = 0;    // the  winding number counter

	// loop through all edges of the polygon
	for (int i = 0; i<V.size(); i++) {   // edge from V[i] to  V[i+1]
		if (V[i].y <= P.y) {                        // start y <= P.y
			if (V[(i + 1) % V.size()].y  > P.y)                  // an upward crossing
				if (isLeft(V[i], V[(i + 1) % V.size()], P) > 0)  // P left of edge
					++wn;                           // have a valid up intersect
		} else {                                    // start y > P.y (no test needed)
			if (V[(i + 1)%V.size()].y <= P.y)                  // a downward crossing
				if (isLeft(V[i], V[(i + 1) % V.size()], P) < 0)  // P right of edge
					--wn;                           // have a valid down intersect
		}
	}
	return wn;
}

// Euclidean distance
inline float dist(Vec a, Vec b) {
	return sqrt((a - b).dot(a - b));
}

inline float perp(Vec u, Vec v) {
	return u.x*v.y - u.y*v.x;
}

template<typename T>
inline T min(T a, T b) {
	return a < b ? a : b;
}

// intersect2D_2Segments(): find the 2D intersection of 2 finite segments
//    Input:  two finite segments S1 and S2
//    Output: *I0 = intersect point (when it exists)
//            *I1 =  endpoint of intersect segment [I0,I1] (when it exists)
//    Return: 0=disjoint (no intersect)
//            1=intersect  in unique point I0
//            2=overlap  in segment from I0 to I1
float raycast_to_Segment(Ray R, Segment S2) {
	Vec R2 = R.point + Vec{ cos(R.theta)*100.0f, sin(R.theta)*100.0f };
	Vec u = R2 - R.point;
	Vec v = S2.P1 - S2.P0;
	Vec w = R.point - S2.P0;
	float D = perp(u, v);

	// test if  they are parallel (includes either being a point)
	if (fabs(D) < 1e-9) {           // S1 and S2 are parallel
		if (perp(u, w) != 0 || perp(v, w) != 0) {
			return -1;                    // they are NOT collinear
		}
		// they are collinear segments - get  overlap (or not)
		float t0, t1;                    // endpoints of S1 in eqn for S2
		Vec w2 = R2 - S2.P0;
		if (v.x != 0) {
			t0 = w.x / v.x;
			t1 = w2.x / v.x;
		}
		else {
			t0 = w.y / v.y;
			t1 = w2.y / v.y;
		}
		if (t0 > t1) {                   // must have t0 smaller than t1
			float t = t0; t0 = t1; t1 = t;    // swap if not
		}
		if (t0 > 1 || t1 < 0) {
			return -1;      // NO overlap
		}
		t0 = t0<0 ? 0 : t0;               // clip to min 0
		t1 = t1>1 ? 1 : t1;               // clip to max 1
		if (t0 == t1) {                  // intersect is a point
			Vec I0 = S2.P0 + t0 * v;
			return dist(I0,R.point);
		}

		// they overlap in a valid subsegment
		Vec I0 = S2.P0 + t0 * v;
		Vec I1 = S2.P0 + t1 * v;
		return min(dist(I0,R.point),dist(I1,R.point));
	}

	// the segments are skew and may intersect in a point
	// get the intersect parameter for S1
	float     sI = perp(v, w) / D;
	if (sI < 0 || sI > 1)                // no intersect with S1
		return -1;

	// get the intersect parameter for S2
	float     tI = perp(u, w) / D;
	if (tI < 0 || tI > 1)                // no intersect with S2
		return -1;

	Vec I0 = R.point + sI * u;                // compute S1 intersect point
	return dist(I0,R.point);
}

//http://geomalgorithms.com/a02-_lines.html#Distance-to-Infinite-Line
// dist_Point_to_Segment(): get the distance of a point to a segment
//     Input:  a Point P and a Segment S (in any dimension)
//     Return: the shortest distance from P to S
float dist_Point_to_Ray(Vec P, Ray R) {
	Vec R2 = R.point + Vec{cos(R.theta)*100.0f, sin(R.theta)*100.0f};
	Vec v = R2 - R.point;
	Vec w = P - R.point;

	double c1 = w.dot(v);
	if (c1 < 0)
		return -1;

	double c2 = v.dot(v);

	double b = c1 / c2;
	Vec Pb = R.point + b * v;
	return dist(P, Pb);
}

typedef struct DistResult {
	float dist;
	Vec pt;
};

// dist_Point_to_Segment(): get the distance of a point to a segment
// Input:  a Point P and a Segment S (in any dimension)
// Return: the shortest distance from P to S
DistResult dist_Point_to_Segment(Vec P, Segment S) {
	Vec v = S.P1 - S.P0;
	Vec w = P - S.P0;

	double c1 = w.dot(v);
	if (c1 <= 0)
		return { dist(P, S.P0),S.P0 };

	double c2 = v.dot(v);
	if (c2 <= c1)
		return { dist(P, S.P1),S.P1 };

	double b = c1 / c2;
	Vec Pb = S.P0 + b * v;
	return { dist(P, Pb), Pb };
}

float raycast(Room room, Ray ray) {
	float closest = 999999999999999;
	for (Obstacle obs : room.obstacles) {
		float dis = dist_Point_to_Ray(obs.loc, ray);
		if (dis > 0 && dis < obs.r) {
			float dis2 = dist(obs.loc, ray.point);
			if (dis2 < closest) {
				closest = dis2;
			}
			
		}
	}
	for (int i = 0; i < room.corners.size(); ++i) {
		Segment s = { room.corners[i],room.corners[(i + 1) % room.corners.size()] };
		float dist = raycast_to_Segment(ray, s);
		if (dist > 0 && dist < closest) {
			closest = dist;
		}
	}
	return closest;
}

Vec rotate(Vec vec, float theta) {
	return{ vec.x*cos(theta) - vec.y*sin(theta),vec.x*sin(theta) + vec.y*cos(theta) };
}



DistResult dist_point_to_room(Vec loc, Room room) {
	float closest = 99999999999999.0f;
	Vec pt = { 0,0 };
	for (Obstacle obs : room.obstacles) {
		float dis = dist(obs.loc, loc) - obs.r;
		if (dis < closest) {
			closest = dis;
			pt = obs.loc;
		}
	}
	for (int i = 0; i < room.corners.size(); ++i) {
		Segment s = { room.corners[i],room.corners[(i + 1) % room.corners.size()] };
		auto res = dist_Point_to_Segment(loc, s);
		if (res.dist > 0 && res.dist < closest) {
			closest = res.dist;
			pt = res.pt;
		}
	}
	return{ closest,pt };
}

vector<DistResult> dist_points_to_room(Vec loc, Room room) {
	float closest = 99999999999999.0f;
	vector<DistResult> res;
	Vec pt = { 0,0 };
	for (Obstacle obs : room.obstacles) {
		float dis = dist(obs.loc, loc) - obs.r;
		res.push_back({ dis,obs.loc });
	}
	for (int i = 0; i < room.corners.size(); ++i) {
		Segment s = { room.corners[i],room.corners[(i + 1) % room.corners.size()] };
		res.push_back(dist_Point_to_Segment(loc, s));
		
	}
	return res;
}

// https://chess.eecs.berkeley.edu/eecs149/documentation/differentialDrive.pdf
void apply_command(Roomba &rmba, const RoombaCommand c) {
	// Do the differential drive stuff
	// Wheels are 0.15 from the center
	const float l = 0.15f;
	const float dt = 0.01f; // 10ms tick
	float omega = (c.v_r - c.v_l) / l;
	float R = l / 2 * (c.v_l + c.v_r) / (c.v_r - c.v_l);
	Vec ICC = { rmba.loc.x - R*sin(rmba.theta),rmba.loc.y + R*cos(rmba.theta) };
	float x = rmba.loc.x;
	float y = rmba.loc.y;
	// Equation 5, expanded out
	if (omega != 0) {
		rmba.loc.x = cos(omega*dt)*(x - ICC.x) - sin(omega*dt)*(y - ICC.y) + ICC.x;
		rmba.loc.y = sin(omega*dt)*(x - ICC.x) + cos(omega*dt)*(y - ICC.y) + ICC.y;
	} else {
		rmba.loc.x = x + cos(rmba.theta)*c.v_r*dt;
		rmba.loc.y = y + sin(rmba.theta)*c.v_r*dt;
	}
	rmba.theta += omega*dt;
}

SensorState senseWorld(Roomba &roomba, const Room room) {
	SensorState ss;
	// The wall sensors point 20 degrees away from the roomba and are
	// located at [0.1,+-0.1] when the roomba is pointed to the right
	// (theta=0).
	Vec left_ray_pt = rotate({ 0.1f,0.1f }, roomba.theta) + roomba.loc;
	Ray left_ray = { left_ray_pt,roomba.theta + 20.0f / 180 * PI };
	ss.wall_left = raycast(room, left_ray);
	Vec right_ray_pt = rotate({ 0.1f,-0.1f }, roomba.theta) + roomba.loc;
	Ray right_ray = { right_ray_pt,roomba.theta - 20.0f / 180 * PI };
	ss.wall_right = raycast(room, right_ray);
	DistResult centerDist = dist_point_to_room(roomba.loc, room);
	if (centerDist.dist < roomba.r) {
		float angle = atan2(centerDist.pt.y - roomba.loc.y, centerDist.pt.x - roomba.loc.x) - roomba.theta;
		if (angle < -60.0f / 180 * PI && angle > -90.0f / 180 * PI) {
			ss.right = true;
			ss.left = false;
			ss.center = false;
		}
		else if (angle > 60.0f / 180 * PI && angle < 90.0f / 180 * PI) {
			ss.left = true;
			ss.right = false;
			ss.center = false;
		}
		else if (angle > -90.0f / 180 * PI && angle < 90.0f / 180 * PI) {
			ss.center = true;
			ss.right = false;
			ss.left = false;
		}
	}
	else {
		ss.left = false;
		ss.right = false;
		ss.center = false;
	}
	return ss;
}

void apply_physics(Roomba &roomba, const Room room) {
	vector<DistResult> centerDist = dist_points_to_room(roomba.loc, room);
	for (auto dr : centerDist) {
		// Collision detection
		if (dr.dist < roomba.r) {
			roomba.loc = roomba.loc - (dr.pt - roomba.loc)*(roomba.r - dr.dist);
		}
	}
}

void simulate(Roomba &roomba,const Room room) {
	SensorState ss = senseWorld(roomba, room);
	auto command = roomba(ss);
	apply_command(roomba,command);
	apply_physics(roomba, room);
}
