#pragma once

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec3 texUV;
};

class VBO
{
public:
	GLuint ID;

	VBO(std::vector<Vertex>& vertices);

	void Bind();

	void Unbind();

	void Delete();
};

class VBOSimple
{
public:
	GLuint ID;

	VBOSimple(GLfloat* vertices, GLsizeiptr size);

	void Bind();

	void Unbind();

	void Delete();
};