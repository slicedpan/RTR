#include "CubeMap.h"
#include <cstdio>
#include "stb_image.h"
#include <GL/glew.h>
#include <cstdlib>

CubeMap::CubeMap(char * filename, int faceWidth) 
	: Texture(filename),
	faceWidth(faceWidth)
{
	format = VERTICALCROSS;
}

CubeMap::CubeMap(char* filename, int faceWidth, int format)
	: Texture(filename),
	faceWidth(faceWidth),
	format(format)
{
}

CubeMap::~CubeMap(void)
{
}

void CubeMap::LoadFromFile()
{
	switch(format)
	{
	case VERTICALCROSS:
		LoadVerticalCross();
		break;
	case HORIZONTALCROSS:
		LoadHorizontalCross();
		break;
	case MULTITEXTURE:
		LoadMultiTexture();
		break;
	}
}

void CubeMap::LoadMultiTexture()
{
	char file[6][128];
	for (int i = 0; i< 6; i++)
	{
		strcpy(file[i], filename);
	}
	strcat(file[0], "/posx.jpg");
	strcat(file[1], "/negx.jpg");
	strcat(file[2], "/posy.jpg");
	strcat(file[3], "/negy.jpg");
	strcat(file[4], "/posz.jpg");
	strcat(file[5], "/negz.jpg");

	glGenTextures(1, &glID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, glID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

	for (int i = 0; i < 6; ++i)
	{
		FILE* inputFile = fopen(file[i], "rb");
		if (!inputFile)
			continue;
		imgData = stbi_load_from_file(inputFile, &width, &height, &components, 4);
		gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
		fclose(inputFile);
		stbi_image_free(imgData);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeMap::LoadHorizontalCross()
{
	
}

void CubeMap::LoadVerticalCross()
{
	FILE* file = fopen(this->filename, "rb");
	imgData = stbi_load_from_file(file, &width, &height, &components, 4);
	fclose(file);

	unsigned char* pixels_face0 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);
	unsigned char* pixels_face1 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);
	unsigned char* pixels_face2 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);
	unsigned char* pixels_face3 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);
	unsigned char* pixels_face4 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);
	unsigned char* pixels_face5 = (unsigned char*)malloc(sizeof(unsigned char) * faceWidth * faceWidth * 4);

	if (width != faceWidth * 3)
		throw;
	if (height != faceWidth * 4)
		throw;

	int stripSize = faceWidth * 4;	
	int imageByteWidth = width * 4;
	for (int i = 0; i < faceWidth; ++i)
	{
		memcpy(pixels_face0 + (i * stripSize), imgData + stripSize + (i * imageByteWidth), sizeof(unsigned char) * stripSize);
	}

	int offset = 0;

	for (int i = faceWidth; i < faceWidth * 2; ++i)
	{
		memcpy(pixels_face1 + (offset * stripSize), imgData + (i * imageByteWidth), sizeof(unsigned char) * stripSize);
		memcpy(pixels_face2 + (offset * stripSize), imgData + (i * imageByteWidth) + stripSize, sizeof(unsigned char) * stripSize);
		memcpy(pixels_face3 + (offset * stripSize), imgData + (i * imageByteWidth) + (2 * stripSize), sizeof(unsigned char) * stripSize);
		++offset;
	}

	offset = 0;

	for (int i = faceWidth * 2; i < faceWidth * 3; ++i)
	{
		memcpy(pixels_face4 + (offset * stripSize), imgData + (i * imageByteWidth) + stripSize, sizeof(unsigned char) * stripSize);
		++offset;
	}

	offset = 0;

	for (int i = faceWidth * 3; i < faceWidth * 4; ++i)
	{
		memcpy(pixels_face5 + (offset * stripSize), imgData + stripSize + (i * imageByteWidth), sizeof(unsigned char) * stripSize);
		++offset;
	}

	glGenTextures(1, &glID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, glID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face1);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face2);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face4);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, faceWidth, faceWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_face5);

	free(pixels_face0);
	free(pixels_face1);
	free(pixels_face2);
	free(pixels_face3);
	free(pixels_face4);
	free(pixels_face5);

	stbi_image_free(imgData);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
