#include "vec3.h"

Vec3 add(Vec3 *v1, Vec3 *v2)
{
	return (Vec3) {v1->x + v2->x, v1->y + v2->y, v1->z + v2->z};
}

Vec3 sub(Vec3 *v1, Vec3 *v2)
{
	return (Vec3) {v1->x - v2->x, v1->y - v2->y, v1->z - v2->z};
}

double dot(Vec3 *v1, Vec3 *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

Vec3 norm(Vec3 *v1)
{
	double m = mag(v1);
	return (Vec3) {v1->x / m, v1->y / m, v1->z / m};
}

double mag(Vec3 *v1)
{
	return sqrt(dot(v1, v1));
}

Vec3 scale(Vec3 *v1, double s)
{
	return (Vec3) {v1->x * s, v1->y * s, v1->z * s};
}
