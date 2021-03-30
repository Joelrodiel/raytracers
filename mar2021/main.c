#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gifenc.h"
#include "parser.h"

#define DBL_MAX 1.7976931348623158e+308

enum { WIDTH = 800, HEIGHT = 600 };
const double ASR = (double)WIDTH/(double)HEIGHT;
const double FOV = 1.5708;

const double DARKEST = 0.5;

Ray newRay(int x, int y);

int rayHit(Ray *r, Object *objs, int objsLen, double *t, int once);

int hitSphere(Sphere *s, Ray *r, double *t);
int hitPlane(Plane *p, Ray *r, double *t);

Vec3 getNormal(Object *obj, Vec3 *hitP);

void swap(double *a, double *b);
uint8_t getNearestSafeColor(Vec3 *c, Vec3 *err);
void applyDithering(Vec3 *bufferIn, uint8_t *bufferOut);

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		printf("No scene file specified: 'rays scene.sc'. Quitting...\n");
		return 1;
	}

	Scene sc = {NULL, 0, {{0.0, 0.0, 0.0}, 0.0},
				(int)WIDTH, (int)HEIGHT, ASR, FOV, DARKEST};

	parseScene(argv[1], &sc);

	char *outPath = "rays.gif";

	if (argc >= 3)
		outPath = argv[2];

	ge_GIF *gif = ge_new_gif(outPath, sc.WIDTH, sc.HEIGHT, NULL, 8, 0);
	
	// for (int i = 0; i < 125; i++)
	// {
		// worldObjs[0].obj.sp.o.y = sin(i * 0.15);
		// worldObjs[1].obj.sp.o.x = 5.0 + sin(i * 0.2);
		// worldObjs[1].obj.sp.o.z = -10.0 - sin(i * 0.1);

		for (int y = 0; y < sc.HEIGHT; y++)
		{
			for (int x = 0; x < sc.WIDTH; x++)
			{
				
				Ray r = newRay(x, y);

				double t = 0.0;
				int objI = rayHit(&r, sc.objs, sc.objsLen, &t, 0);
				if (objI >= 0)
				{
					Vec3 rDist = scale(&r.d, t);
					Vec3 hitP = add(&r.o, &rDist);

					Vec3 newDir = sub(&sc.li.o, &hitP);
					double lightMag = mag(&newDir);
					newDir = norm(&newDir);
					Vec3 objNorm = getNormal(&(sc.objs[objI]), &hitP);
					// Vec3 rayO = scale(&objNorm, 1e-4);
					// rayO = add(&rayO, &hitP);
					// Ray shadowRay = {rayO, newDir};

					// double p = 0.0;
					// if (rayHit(&shadowRay, worldObjs, worldObjsLen, &p, 1) >= 0 && lInt >= DARKEST)
					// {
					// 	buffer[x+(WIDTH*y)] = 245;
					// }
					// else
					// {
						double lInt = dot(&objNorm, &newDir) * sc.li.r / pow(lightMag, 2.0);
						lInt = (lInt >= DARKEST) ? lInt : DARKEST;
						lInt = (lInt > 1) ? 1 : lInt;
						Vec3 col = scale(&(sc.objs[objI].color), lInt);
						gif->frame[x + (WIDTH * y)] = getNearestSafeColor(&col, NULL);
					// }
				}
				else
				{
					gif->frame[x + (WIDTH * y)] = 0;
				}
			}
		}

		ge_add_frame(gif, 7);
	// }

	ge_close_gif(gif);

	free(sc.objs);

	return 0;
}

Ray newRay(int x, int y)
{
	double fov = tan(FOV / 2.0);
	double rX = ((((double)x + 0.5) / (double)WIDTH) * 2.0 - 1.0) * ASR * fov;
	double rY = (1.0 - (((double)y + 0.5) / (double)HEIGHT) * 2.0) * fov;

	Vec3 dir = {rX, rY, -1.0};

	return (Ray) {{0.0, 0.0, 0.0}, norm(&dir)};
}

int rayHit(Ray *r, Object *objs, int objsLen, double *t, int once)
{
	double t0 = 0.0;
	double big = DBL_MAX;
	int objI = -1;

	for (int i = 0; i < objsLen; i++)
	{
		int hit = 0;

		switch (objs[i].type)
		{
			// Sphere intersection
			case 0:
				hit = hitSphere(&objs[i].obj.sp, r, &t0);
				break;
			// Plane intersection
			case 1:
				hit = hitPlane(&objs[i].obj.pl, r, &t0);
				break;
			default:
				break;
		}

		if (hit && t0 < big)
		{
			big = t0;
			objI = i;
			if (once)
				break;
		}
	}

	*t = big;
	return objI;
}

int hitSphere(Sphere *s, Ray *r, double *t)
{
	Vec3 a = sub(&s->o, &r->o);
	double b = dot(&a, &r->d);
	double c = dot(&a, &a) - pow(b, 2.0);
	double r2 = pow(s->r, 2.0);

	if (c > r2)
		return 0;
	
	double tH = sqrt(r2 - c);
	double t0 = b - tH, t1 = b + tH;

	if (t0 > t1)
		swap(&t0, &t1);

	if (t0 < 0.0)
	{
		t0 = t1;
		if (t0 < 0.0)
			return 0;
	}

	*t = t0;
	
	return 1;
}

int hitPlane(Plane *p, Ray *r, double *t)
{
	double denom = dot(&p->n, &r->d);

	if (denom > 1e-6)
	{
		Vec3 p0 = sub(&p->o, &r->o);
		*t = dot(&p0, &p->n) / denom;
		return (t != NULL && *t >= 0);
	}

	return 0;
}

Vec3 getNormal(Object *obj, Vec3 *hitP)
{
	Vec3 out = {0.0, 0.0, 0.0};

	switch (obj->type)
	{
		// Sphere Normal
		case 0:
			out = sub(hitP, &(obj->obj.sp.o));
			out = norm(&out);
			break;
		// Plane normal
		case 1:
			out = obj->obj.pl.n;
			break;
		default:
			break;
	}

	return out;
}

void swap(double *a, double *b)
{
	double t;

	t  = *b;
	*b = *a;
	*a = t;
}

uint8_t getNearestSafeColor(Vec3 *c, Vec3 *err)
{
	uint8_t r = (uint8_t)(round(c->x * 5) * 51);
	uint8_t g = (uint8_t)(round(c->y * 5) * 51);
	uint8_t b = (uint8_t)(round(c->z * 5) * 51);
	
	Vec3 newCol = {(float)r, (float)g, (float)b};
	if (err != NULL)
		*err = sub(c, &newCol);

	return (((uint8_t)newCol.z/0x33) + ((uint8_t)newCol.y/0x33)*6 + ((uint8_t)newCol.x/0x33)*36 + 16) % 256;
}

// void applyDithering(Vec3 *bufferIn, uint8_t *bufferOut)
// {
// 	int MAX = WIDTH*HEIGHT;
// 	Vec3 err = {0.0, 0.0, 0.0};
// 	for (int y = 0; y < HEIGHT; y++)
// 	{
// 		for (int x = 0; x < WIDTH; x++)
// 		{
// 			bufferOut[x + (WIDTH * y)] = getNearestSafeColor(&bufferIn[x + (WIDTH * y)], &err);

// 			Vec3 quantErr = scale(&err, 7.0/16.0);
// 			int index = (x + 1) + (WIDTH * y);
// 			Vec3 palCol = add(&bufferIn[index], &quantErr);
// 			if (index < MAX)
// 			{
// 				bufferIn[index] = (Vec3){palCol.x, palCol.y, palCol.z};
// 				// bufferOut[index] = getNearestSafeColor(&palCol, NULL);
// 			}
// 			quantErr = scale(&err, 3.0/16.0);
// 			index = (x - 1) + (WIDTH * (y + 1));
// 			if (index < MAX)
// 			{
// 				bufferIn[index] = add(&bufferIn[index], &quantErr);
// 				// bufferIn[index] = (Vec3){palCol.x, palCol.y, palCol.z};
// 				// bufferOut[index] = getNearestSafeColor(&palCol, NULL);
// 			}
// 			quantErr = scale(&err, 5.0/16.0);
// 			index = x + (WIDTH * (y + 1));
// 			if (index < MAX)
// 			{
// 				bufferIn[index] = add(&bufferIn[index], &quantErr);
// 				// bufferIn[index] = (Vec3){palCol.x, palCol.y, palCol.z};
// 				// bufferOut[index] = getNearestSafeColor(&palCol, NULL);
// 			}
// 			quantErr = scale(&err, 1.0/16.0);
// 			index = (x + 1) + (WIDTH * (y + 1));
// 			if (index < MAX)
// 			{
// 				bufferIn[index] = add(&bufferIn[index], &quantErr);
// 				// bufferIn[index] = (Vec3){palCol.x, palCol.y, palCol.z};
// 				// bufferOut[index] = getNearestSafeColor(&palCol, NULL);
// 			}
// 		}
// 	}
// }
