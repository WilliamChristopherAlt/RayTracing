#pragma once

#include <glad/glad.h>
#include <OpenGL/textureClass.h>

class FBO
{
public:
    GLuint ID;

    FBO(Texture2D texture);

    void Bind();
    void Unbind();
    void Delete();
};
