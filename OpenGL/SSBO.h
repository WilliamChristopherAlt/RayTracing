#pragma once

#include<glad/glad.h>

class SSBO
{
public:
    GLuint ID;
    GLuint bindingIndex; // The global binding index of the SSBO across all shaders

    SSBO(void* data, GLsizeiptr size, GLuint bindIndex);

    void Bind();
    void Unbind();
    void Delete();
};
