#pragma once

#include "Texture.h"
#define VERTICALCROSS 0
#define HORIZONTALCROSS 1
#define MULTITEXTURE 2

class CubeMap : public Texture
{
public:
	CubeMap(char * filename, int faceWidth);
	CubeMap(char * filename, int faceWidth, int format);
	~CubeMap(void);
protected:
	void LoadFromFile();
	void LoadVerticalCross();
	void LoadHorizontalCross();
	void LoadMultiTexture();
private:	
	int faceWidth;
	int format;
};

