#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define to_radians(angle) ((angle) * M_PI / 180.0)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum { WIDTH = 800, HEIGHT = 600 };
const double FOV = 90.0;
const double A_R = (double)WIDTH/(double)HEIGHT;

struct Vec3
{
	double x;
	double y;
	double z;
};

struct Vec3 vec3_sub(struct Vec3 *a, struct Vec3 *b)
{
	struct Vec3 out = {
		a->x - b->x, a->y - b->y, a->z - b->z
	};
	return out;
}

double vec3_dot(struct Vec3 *a, struct Vec3 *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

double vec3_mag(struct Vec3 *a)
{
	return vec3_dot(a, a);
}

struct Vec3 vec3_norm(struct Vec3 *a)
{
	double magn = vec3_mag(a);
	struct Vec3 out = {a->x / magn, a->y / magn, a->z / magn};
	return out;
}

struct Ray
{
	struct Vec3 o;
	struct Vec3 d;
};

struct Sphere
{
	struct Vec3 o;
	double r;
};

struct Ray create_ray(int x, int y);

int sphere_hit(struct Ray *r, struct Sphere *sp, double *t);

int main(void)
{
	unsigned char data[WIDTH*HEIGHT*3+(WIDTH*4)];
	struct Sphere sp = {{0.0, 0.0, -5.0}, 3.0};

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			struct Ray ray = create_ray(x, y);

			int pos = (WIDTH*3 * y) + (3 * x);

			double t;

			if (sphere_hit(&ray, &sp, &t) == 1)
			{
				data[pos + 0] = 200;
				data[pos + 1] = 100;
				data[pos + 2] = 100;
			}
		}
	}

	stbi_write_png("out.png", WIDTH, HEIGHT, 3, data, WIDTH*3);

	return 0;
}

struct Ray create_ray(int x, int y)
{
	double fov = tan(to_radians(FOV) / 2.0);
	double r_x = ((((double)x + 0.5) / WIDTH) * 2 - 1) * A_R * fov;
	double r_y = (1 - (((double)y + 0.5) / HEIGHT) * 2) * fov;

	struct Vec3 dir = {
		r_x, r_y, -1.0
	};

	struct Ray out = {
		.d = vec3_norm(&dir)
	};

	return out;
}

int sphere_hit(struct Ray *r, struct Sphere *sp, double *t)
{
	struct Vec3 L = vec3_sub(&sp->o, &r->o);
	double t_h    = vec3_dot(&L, &r->d);
	double r2	  = sp->r * sp->r;
	double d2     = vec3_mag(&L) - t_h*t_h;
	if (d2 > r2)
	{
		return 0;
	}
	double t_hc = sqrt(r2 - d2);
	double t0 = t_h - t_hc, t1 = t_h + t_hc;

	if (t0 < t1) swap(t0, t1);

	if (t0 < 0)
	{
		t0 = t1;
		if (t0 < 0) return 0;
	}

	*t = t0;

	return 1;
}
