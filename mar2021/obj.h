#ifndef OBJ_H
#define OBJ_H
#include "vec3.h"

typedef struct Ray {
	Vec3 o;
	Vec3 d;
} Ray;

typedef struct Sphere {
	Vec3 o;
	double r;
} Sphere;

typedef struct Plane {
	Vec3 o;
	Vec3 n;
} Plane;

typedef Sphere Light;

typedef struct Object {
	int type;
	Vec3 color;
	union
	{
		Sphere sp;
		Plane pl;
	} obj;
} Object;

#endif