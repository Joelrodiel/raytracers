#ifndef VEC3_H
#define VEC3_H
#include <math.h>

typedef struct Vec3 {
	double x;
	double y;
	double z;
} Vec3;

Vec3 sub(Vec3 *v1, Vec3 *v2);
Vec3 add(Vec3 *v1, Vec3 *v2);
double dot(Vec3 *v1, Vec3 *v2);
Vec3 norm(Vec3 *v1);
double mag(Vec3 *v1);
Vec3 scale(Vec3 *v1, double s);

#endif