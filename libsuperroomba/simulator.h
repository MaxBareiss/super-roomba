#pragma once

#ifdef LIBSUPERROOMBA_EXPORTS
#define LIBSUPERROOMBA_API __declspec(dllexport) 
#else
#define LIBSUPERROOMBA_API __declspec(dllimport) 
#endif

#include <vector>
#include "Roomba.h"
#include "Types.h"

#define PI 3.141592653589793f

using namespace std;

// Everything is in MKS

LIBSUPERROOMBA_API Vec operator* (float lhs, Vec rhs);

LIBSUPERROOMBA_API void simulate(Roomba &roomba, Room room);
LIBSUPERROOMBA_API float dist_Point_to_Ray(Vec P, Ray R);
LIBSUPERROOMBA_API float raycast_to_Segment(Ray R, Segment S2);
LIBSUPERROOMBA_API Vec rotate(Vec vec, float theta);
LIBSUPERROOMBA_API inline float dist(Vec a, Vec b);