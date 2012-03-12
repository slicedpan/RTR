#include "Uniform.h"
#include <svl\SVL.h>
#include <GL\glew.h>

Uniform::Uniform(std::string name, unsigned int id)
	: name(name),
	id(id)
{
}

Uniform::~Uniform(void)
{
}

void Uniform::SetValue(Mat4& matrix)
{
	if (id >= 0)
		glUniformMatrix4fv(id, 1, false, matrix.Ref());
}
void Uniform::SetValue(Mat3& matrix)
{
	if (id >= 0)
		glUniformMatrix3fv(id, 1, false, matrix.Ref());
}
void Uniform::SetValue(Vec3& vector)
{
	if(id >= 0)
		glUniform3fv(id, 1, vector.Ref());
}
void Uniform::SetValue(Vec4& vector)
{
	if (id >= 0)
		glUniform4fv(id, 1, vector.Ref());
}

void Uniform::SetValue(int intVal)
{
	if (id >= 0)
		glUniform1i(id, intVal);
}

void Uniform::SetValue(float floatVal)
{
	if (id >= 0)
		glUniform1f(id, floatVal);
}

