#pragma once

#include <string>

class Mat4;
class Mat3;
class Vec3;
class Vec4;

class Uniform
{
public:
	Uniform(std::string name, unsigned int id);
	Uniform() : name(""), id(-1) {}
	~Uniform(void);
	void SetValue(Mat4& matrix);
	void SetValue(Mat3& matrix);
	void SetValue(Vec3& vector);
	void SetValue(Vec4& vector);
	void SetValue(int intVal);
	void SetValue(float floatVal);
	std::string& GetName() { return name; }
	unsigned int GetId() { return id; }
private:
	std::string name;
	int id;
};

