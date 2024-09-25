#pragma once

#include<glad/glad.h>

class UBO
{
public:
    GLuint ID;
    GLuint bindingIndex; // The global binding index of the UBO across all shaders

    UBO(GLuint bindIndex, GLsizeiptr size);
    
    void Update(void* data, GLsizeiptr size);

    void Bind();
    void Unbind();
    void Delete();
};
