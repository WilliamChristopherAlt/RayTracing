#include <OpenGL/textureClass.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Constructor to generate the texture and set its parameters
Texture2D::Texture2D(int width, int height, const void* pixels, int mipmapLevel, GLenum pixelFormat, GLint filterMode, GLint wrapMode, GLenum textureUnit)
{
    unit = textureUnit;
    glActiveTexture(unit);
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGB, width, height, 0, GL_RGB, pixelFormat, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::Texture2D(int width, int height, GLenum textureUnit)
{
    unit = textureUnit;
    glActiveTexture(unit);
    glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::Texture2D(const std::string& path, GLenum textureUnit)
{
    unit = textureUnit;
    glActiveTexture(unit);
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    int width, height, numColCh;
    // stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &numColCh, 0);
    if (pixels)
    {
        std::cout << "Texture's number of color channels: " << numColCh << std::endl;
        unsigned char* rgbPixels = new unsigned char[width * height * 4];
        for (int i = 0; i < width * height; i++)
        {
            rgbPixels[4 * i    ] = pixels[numColCh * i    ];
            rgbPixels[4 * i + 1] = pixels[numColCh * i + 1];
            rgbPixels[4 * i + 2] = pixels[numColCh * i + 2];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA , GL_UNSIGNED_BYTE, rgbPixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_image_free(pixels);
        delete[] rgbPixels;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        throw(errno);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec3 Texture2D::readPixel(const glm::vec2& uv)
{
    GLubyte pixel[3];
    glBindTexture(GL_TEXTURE_2D, ID);
    glReadPixels(uv.x, uv.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    return glm::vec3(pixel[0], pixel[1], pixel[2]);
}

void Texture2D::Bind()
{
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture2D::SetActive()
{
    glActiveTexture(unit);
}

void Texture2D::Unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Delete()
{
    glDeleteTextures(1, &ID);
}
