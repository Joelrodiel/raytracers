#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

Object parseObject(char *obj);

int getObjectCount(FILE *f);

int parseScene(char *fileName, Scene *s)
{
	FILE *f = fopen(fileName, "r");

	int objCount = getObjectCount(f);

	if (objCount > 0 && s != NULL)
	{
		s->objs = malloc(sizeof(Object) * objCount);
		s->objsLen = objCount;
	}

	char token;
	int objNum = 0;

	while ((token = (char)fgetc(f)) != EOF)
	{
		char line[255];
		fgets(line, 255, f);

		if (token >= 95 && token <= 122)
		{
			switch (token)
			{
				case 'o': ;
					Object obj = parseObject(line);
					if (s != NULL)
						s->objs[objNum++] = obj;
					break;
				case 'l': ;
					Vec3 o = {0.0, 0.0, 0.0};
					double i = 0.0;
					sscanf(line, " %lf,%lf,%lf,%lf", &o.x, &o.y, &o.z, &i);
					s->li = (Light){o, i};
					break;
				case 's':
					sscanf(" %d,%d,%lf,%lf", &s->WIDTH, &s->HEIGHT, &s->FOV, &s->DARKEST);
					printf("Wow: %d,%d,%f,%f", s->WIDTH, s->HEIGHT, s->FOV, s->DARKEST);
					s->AsR = (double)s->WIDTH / (double)s->HEIGHT;
					break;
				default:
					printf("Warning: Token '%c' not recognized.\n", token);
					break;
			}
		}
	}

	fclose(f);

	return 1;
}

Object parseObject(char *obj)
{
	char type;
	sscanf(obj, " %c,", &type);
	
	Vec3 c = {0.0, 0.0, 0.0};
	Vec3 o = {0.0, 0.0, 0.0};
	Object out = {0, {}, {}};

	switch (type)
	{
		case 's': ;
			double r = 0.0;
			sscanf(obj, " %c,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &type, &c.x, &c.y, &c.z, &o.x, &o.y, &o.z, &r);
			out.obj.sp = (Sphere) {o, r};
			break;
		case 'p': ;
			Vec3 n = {0.0, 0.0, 0.0};
			sscanf(obj, " %c,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &type, &c.x, &c.y, &c.z, &o.x, &o.y, &o.z, &n.x, &n.y, &n.z);
			out.type = 1;
			out.obj.pl = (Plane) {o, n};
			break;
		default:
			printf("Error: Object '%c' not recognized.\n", type);
			break;
	}

	out.color = c;

	return out;
}

int getObjectCount(FILE *f)
{
	int count = 0;
	char ch = (char)fgetc(f);

	if (ch == 'o')
		count++;

	while ((ch = (char)fgetc(f)) != EOF)
	{
		if (ch == '\n' && (ch = (char)fgetc(f)) == 'o')
			count++;
	}

	rewind(f);

	return count;
}
