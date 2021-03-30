#ifndef PARSER_H
#define PARSER_H
#include "obj.h"

typedef struct Scene {
	Object *objs;
	int objsLen;
	Light li;
	int WIDTH, HEIGHT;
	double AsR, FOV, DARKEST;
} Scene;

int parseScene(char *fileName, Scene *s);

#endif