#pragma once
#include <cstring>

class Texture
{
public:
	Texture(char* filename);
	~Texture(void);
	unsigned int GetId() { return glID; }
	void Load();
protected:
	char filename[128];
	virtual void LoadFromFile();
	unsigned char* imgData;
	int height;
	int width;
	int components;
	unsigned int glID;
};

