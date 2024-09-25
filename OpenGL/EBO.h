#pragma once

#include <glad/glad.h>
#include <vector>

class EBO
{
public:
    GLuint ID;

    EBO(std::vector<GLuint> indicies);

    void Bind();
    void Unbind();
    void Delete();
};
