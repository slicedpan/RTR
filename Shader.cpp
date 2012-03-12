#include "Shader.h"
#include "Uniform.h"
#include <GL\glew.h>
#include <cstdio>
#include <cstdlib>

Uniform Shader::dummy;

Shader::Shader(char * vertexFileName, char * fragmentFileName) : uniformNumber(0)
{	
	SetSource(vertexFileName, fragmentFileName);
	LoadFromFiles();	
}

Shader::Shader(char * vertexFileName, char * fragmentFileName, char* name) : uniformNumber(0)
{
	SetSource(vertexFileName, fragmentFileName);
	LoadFromFiles();
	SetName(name);
}

void Shader::SetSource(char * vertexFileName,char * fragmentFileName)
{
	vFileName = (char*)malloc(strlen(vertexFileName));
	fFileName = (char*)malloc(strlen(fragmentFileName));
	strcpy(vFileName, vertexFileName);
	strcpy(fFileName, fragmentFileName);
}

void Shader::LoadFromFiles()
{	
	vertexID = glCreateShader(GL_VERTEX_SHADER);	
	fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
	char * vertexSource = getSourceFromFile(vFileName);
	char * fragmentSource = getSourceFromFile(fFileName);
	if (!vertexSource || !fragmentSource)
	{
		sprintf(errorLog, "Could not find files %s and %s!", vFileName, fFileName);
		return;
	}
	int length = strlen(vertexSource);
	glShaderSource(vertexID, 1, (const GLchar**)&vertexSource, &length);
	length = strlen(fragmentSource);
	glShaderSource(fragmentID, 1, (const GLchar**)&fragmentSource, &length);
	
	free(vertexSource);
	free(fragmentSource);	
}

void Shader::SetUniforms()
{
	char buf[64];
	int size;
	GLenum type;
	glGetProgramiv(glID, GL_ACTIVE_UNIFORMS, &uniformNumber);
	uniforms.clear();
	for (unsigned int i = 0; i < uniformNumber; ++i)
	{
		glGetActiveUniform(glID, i, 64, 0, &size, &type, buf);
		uniforms.insert(uniformEntry(std::string(buf), Uniform(std::string(buf), i)));
	}
}

bool Shader::Compile()
{
	char buf[256];
	int flag = 0;

	memset((void*)errorLog, 0, sizeof(char) * 1024);

	glID = glCreateProgram();

	glCompileShader(vertexID);
	glGetShaderInfoLog(vertexID, 256, NULL, buf);
	strncat(errorLog, buf, 256);
	glGetShaderiv(vertexID, GL_COMPILE_STATUS, &flag);
	if (!flag)
		return false;
	
	glCompileShader(fragmentID);
	glGetShaderInfoLog(fragmentID, 256, NULL, buf);
	strncat(errorLog, buf, 256);
	glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &flag);
	if (!flag)
		return false;

	glAttachShader(glID, vertexID);
	glAttachShader(glID, fragmentID);
	glLinkProgram(glID);
	glGetProgramInfoLog(glID, 256, NULL, buf);	
	strncat(errorLog, buf, 256);
	glGetProgramiv(glID, GL_LINK_STATUS, &flag);
	if (!flag)
		return false;
	SetUniforms();
	return true;
}

Shader::~Shader(void)
{
}

char * getSourceFromFile(char* filename)
{
	FILE * input = 0;
	input = fopen(filename, "rb");
	if (!input)
		return 0;
	int size;
	char * buf;
	if (!input)
		return 0;
	fseek(input, 0, SEEK_END);
	size = ftell(input);
	fseek(input, 0, SEEK_SET);
	buf = (char*)malloc(sizeof(char) * size);
	fread(buf, sizeof(char), size, input);
	buf[size - 1] = 0;
	fclose(input);
	return buf;
}

void Shader::Use()
{
	glUseProgram(glID);
}

bool Shader::Reload()
{
	glDeleteProgram(glID);
	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);
	LoadFromFiles();
	return Compile();
}

Uniform& Shader::Uniforms(std::string name)
{
	uniformIterator iter = uniforms.find(name);
	if (iter != uniforms.end())
		return iter->second;
	else
		return dummy;
}