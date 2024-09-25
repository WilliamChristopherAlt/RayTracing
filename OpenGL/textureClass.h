#pragma once

#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

class Texture2D
{
public:
    GLuint ID = 0;
    GLenum unit;

    Texture2D(int width, int height, const void* pixels, int mipmapLevel, GLenum pixelFormat, GLint filterMode, GLint wrapMode, GLenum textureUnit);
    Texture2D(const std::string& path, GLenum textureUnit);
    Texture2D(int width, int height, GLenum textureUnit);

    glm::vec3 readPixel(const glm::vec2& uv);

    void SetActive();
    void Bind();
    void Unbind();
    void Delete();
};
