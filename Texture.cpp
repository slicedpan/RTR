#include "Texture.h"
#include "stb_image.h"
#include <GL\glew.h>

Texture::Texture(char* filename) 
	: imgData(0),
	glID(0)
{
	strncpy(this->filename, filename, strlen(this->filename));
}

Texture::~Texture(void)
{
}

void Texture::LoadFromFile()
{
	FILE* file = fopen(this->filename, "rb");
	imgData = stbi_load_from_file(file, &width, &height, &components, 4);
	fclose(file);
	glGenTextures(1, &glID);
	glBindTexture(GL_TEXTURE_2D, glID);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
	stbi_image_free(imgData);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Load()
{
	if (glID > 0)
		glDeleteTextures(1, &glID);
	glID = 0;
	LoadFromFile();
}
