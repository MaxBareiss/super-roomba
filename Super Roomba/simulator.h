#pragma once

#include <vector>
#include "Roomba.h"
#include "Types.h"

#define PI 3.141592653589793f

using namespace std;

// Everything is in MKS

Vec operator* (float lhs, Vec rhs);

void simulate(Roomba &roomba, Room room);
float dist_Point_to_Ray(Vec P, Ray R);
float raycast_to_Segment(Ray R, Segment S2);
Vec rotate(Vec vec, float theta);
inline float dist(Vec a, Vec b);