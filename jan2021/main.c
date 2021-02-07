#include <stdio.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define GAMMA 2.2

#define DBL_MAX 1.7976931348623158e+308
#define RAND_MAX 0x7fff

#define to_radians(angle) ((angle) * M_PI / 180.0)
#define to_rgb(f) (f >= 1.0 ? 255 : (f <= 0.0 ? 0 : (int)floor(f * 256.0)))
#define max(a, b) ((a > b) ? a : b)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum { WIDTH = 800, HEIGHT = 600 };
const double FOV = 90.0;
const double A_R = (double)WIDTH/(double)HEIGHT;
const int    SS  = 10;

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

struct Vec3 vec3_div(struct Vec3 *a, double b)
{
	struct Vec3 out = {
		a->x / b, a->y / b, a->z / b
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

void vec3_apply(struct Vec3 *v, double (*f)(double))
{
	v->x = f(v->x);
	v->y = f(v->y);
	v->z = f(v->z);
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
	struct Vec3 color;
};

struct Ray create_ray(double x, double y);

int sphere_hit(struct Ray *r, struct Sphere *sp, double *t);

struct Vec3 sphere_normal(struct Sphere *sp , struct Vec3 *hit);

int ray_hit(struct Ray *r, struct Sphere *objs[], int objs_len, struct Sphere *obj, double *t);

void swap(double*, double*);

double gamma_encode(double a)
{
	return pow(a, 1.0/GAMMA);
}

void print_vec3(struct Vec3 *a)
{
	printf("[%f, %f, %f]\n", a->x, a->y, a->z);
}

double rand_2()
{
	return (double)rand() / (double)RAND_MAX;
}

int main(void)
{
	unsigned char data[WIDTH*HEIGHT*3+(WIDTH*4)];
	struct Sphere sp = {{-4.0, 0.0, -7.0}, 3.0, {0.78, 0.13, 0.13}};
	struct Sphere sp2 = {{4.0, 0.0, -7.0}, 3.0, {0.13, 0.13, 0.78}};
	struct Sphere light = {{2.0, 0.0, -1.0}, 5.0, {1.0, 1.0, 1.0}};

	struct Sphere *scene_spheres[] = {&sp, &sp2};
	int scene_spheres_len = 2;

	clock_t begin = clock();

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			struct Vec3 out_col = {0.0, 0.0, 0.0};

			// for (int s = 0; s < SS; s++)
			// {
				struct Ray ray = create_ray(x, y);
				// struct Ray ray = create_ray((double)x + rand_2(), (double)y + rand_2());

				struct Sphere obj;
				double t = DBL_MAX;

				if (ray_hit(&ray, scene_spheres, scene_spheres_len, &obj, &t))
				{
					struct Vec3 dist = vec3_scale(&ray.d, t);
					struct Vec3 hit_p = vec3_add(&ray.o, &dist);
					struct Vec3 norml = sphere_normal(&obj, &hit_p);
					struct Vec3 light_v = vec3_sub(&light.o, &hit_p);
					struct Vec3 light_n = vec3_norm(&light_v);
					double l_int = vec3_dot(&norml, &light_n) * light.r / pow(vec3_mag(&light_v), 2.0);
					struct Vec3 col = vec3_scale(&(obj.color), l_int);
					vec3_apply(&col, &gamma_encode);
					out_col = vec3_add(&out_col, &col);
				}
			// }

			// out_col = vec3_div(&out_col, SS);

			int pos = (WIDTH * 3 * y) + (3 * x);

			data[pos + 0] = to_rgb(out_col.x);
			data[pos + 1] = to_rgb(out_col.y);
			data[pos + 2] = to_rgb(out_col.z);
		}
	}

	clock_t end = clock();

	stbi_write_png("out.png", WIDTH, HEIGHT, 3, data, WIDTH*3);

	printf("Time rendering: %f s\n", (double)(end - begin)/CLOCKS_PER_SEC);

	return 0;
}

struct Ray create_ray(double x, double y)
{
	double fov = tan(to_radians(FOV) / 2.0);
	double r_x = (((x + 0.5) / WIDTH) * 2.0 - 1.0) * A_R * fov;
	double r_y = (1.0 - ((y + 0.5) / HEIGHT) * 2.0) * fov;

	struct Vec3 dir = {
		r_x, r_y, -1.0
	};

	struct Ray out = {
		.o = {0.0, 0.0, 0.0},
		.d = vec3_norm(&dir)
	};

	return out;
}

int ray_hit(struct Ray *r, struct Sphere *objs[], int objs_len, struct Sphere *obj, double *t)
{
	int hit = 0;
	double dist = *t;

	for (int i = 0; i < objs_len; i++)
	{
		if (sphere_hit(r, objs[i], &dist) && dist < *t)
		{
			*obj = *objs[i];
			*t = dist;
			hit = 1;
		}
	}

	return hit;
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

struct Vec3 sphere_normal(struct Sphere *sp , struct Vec3 *hit)
{
	struct Vec3 out = vec3_sub(hit, &(sp->o));
	return vec3_norm(&out);
}

void swap(double *a, double *b)
{
	double t;

	t  = *b;
	*b = *a;
	*a = t;
}
