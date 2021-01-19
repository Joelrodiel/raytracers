#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define GAMMA 2.2

#define to_radians(angle) ((angle) * M_PI / 180.0)
#define to_rgb(f) (f >= 1.0 ? 255 : (f <= 0.0 ? 0 : (int)floor(f * 256.0)))
#define max(a, b) ((a > b) ? a : b)

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

struct Vec3 vec3_add(struct Vec3 *a, struct Vec3 *b)
{
	struct Vec3 out = {
		a->x + b->x, a->y + b->y, a->z + b->z
	};
	return out;
}

struct Vec3 vec3_sub(struct Vec3 *a, struct Vec3 *b)
{
	struct Vec3 out = {
		a->x - b->x, a->y - b->y, a->z - b->z
	};
	return out;
}

struct Vec3 vec3_scale(struct Vec3 *a, double b)
{
	struct Vec3 out = {
		a->x * b, a->y * b, a->z * b
	};
	return out;
}

double vec3_dot(struct Vec3 *a, struct Vec3 *b)
{
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

double vec3_mag(struct Vec3 *a)
{
	return sqrt(vec3_dot(a, a));
}

double vec3_mag_sqrd(struct Vec3 *a)
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

struct Vec3 sphere_normal(struct Vec3 *sp_o , struct Vec3 *hit);

void swap(double*, double*);

void print_vec3(struct Vec3 *a)
{
	printf("[%f, %f, %f]\n", a->x, a->y, a->z);
}

int main(void)
{
	unsigned char data[WIDTH*HEIGHT*3+(WIDTH*4)];
	struct Sphere sp = {{0.0, 0.0, -5.0}, 3.0};
	struct Sphere light = {{2.0, 1.0, -1.0}, -5.0};

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			struct Ray ray = create_ray(x, y);

			int pos = (WIDTH*3 * y) + (3 * x);

			double t;

			if (sphere_hit(&ray, &sp, &t))
			{
				struct Vec3 dist = vec3_scale(&ray.d, t);
				struct Vec3 hit_p = vec3_add(&ray.o, &dist);
				struct Vec3 norml = sphere_normal(&sp.o, &hit_p);
				norml.x *= -1;
				norml.y *= -1;
				norml.z *= -1;
				struct Vec3 light_v = vec3_sub(&light.o, &hit_p);
				struct Vec3 light_n = vec3_norm(&light_v);
				double l_int = max(vec3_dot(&norml, &light_n) * light.r / pow(vec3_mag(&light_v), 2.0), 0.1);
				data[pos + 0] = to_rgb((200.0/255.0)*l_int);
				data[pos + 1] = to_rgb((35.0/255.0)*l_int);
				data[pos + 2] = to_rgb((35.0/255.0)*l_int);
			}
		}
	}

	stbi_write_png("out.png", WIDTH, HEIGHT, 3, data, WIDTH*3);

	return 0;
}

struct Ray create_ray(int x, int y)
{
	double fov = tan(to_radians(FOV) / 2.0);
	double r_x = ((((double)x + 0.5) / WIDTH) * 2.0 - 1.0) * A_R * fov;
	double r_y = (1.0 - (((double)y + 0.5) / HEIGHT) * 2.0) * fov;

	struct Vec3 dir = {
		r_x, r_y, -1.0
	};

	struct Ray out = {
		.o = {0.0, 0.0, 0.0},
		.d = vec3_norm(&dir)
	};

	return out;
}

int sphere_hit(struct Ray *r, struct Sphere *sp, double *t)
{
	struct Vec3 L = vec3_sub(&sp->o, &r->o);
	double t_h    = vec3_dot(&L, &r->d);
	double r2	  = pow(sp->r, 2.0);
	double d2     = vec3_mag_sqrd(&L) - pow(t_h, 2.0);
	if (d2 > r2)
	{
		return 0;
	}
	double t_hc = sqrt(r2 - d2);
	double t0 = t_h - t_hc, t1 = t_h + t_hc;

	if (t0 > t1) swap(&t0, &t1);

	if (t0 < 0.0)
	{
		t0 = t1;
		if (t0 < 0.0) return 0;
	}

	*t = t0;

	return 1;
}

struct Vec3 sphere_normal(struct Vec3 *sp_o , struct Vec3 *hit)
{
	struct Vec3 out = vec3_sub(hit, sp_o);
	return vec3_norm(&out);
}

void swap(double *a, double *b)
{
	double t;

	t  = *b;
	*b = *a;
	*a = t;
}
