#include "glmstbi.h"

unsigned char* glmStbiLoad(char* filename, GLboolean alpha, int* width, int* height, int* type)
{
	int components;
	FILE* file;
	//file = fopen(filename, "rb");
	unsigned char* imgData = 0;
	imgData = stbi_load_from_file(file, width, height, &components, 0);
	switch(components)
	{
	case 1:
		*type = GL_LUMINANCE;
		break;
	case 2:
		*type = GL_LUMINANCE_ALPHA;
		break;
	case 3:
		*type = GL_RGB;
		break;
	case 4:
		*type = GL_RGBA;
		break;
	}
	return imgData;
}