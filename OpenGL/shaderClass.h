#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesUtil/myFile.h>
#include <OpenGL/SSBO.h>

void compileErrors(GLuint shader, const char* type);

class Shader
{
public:
    GLuint ID;

    Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath);

    void Activate();
    void Delete();

    void setBool(const char* uniform, bool val);
    void setInt(const char* uniform, int val);
    void setUint(const char* uniform, int val);
    void setFloat(const char* uniform, float val);
    void setVec2(const char* uniform, const glm::vec2& vec);
    void setVec3(const char* uniform, const glm::vec3& vec);
    void setVec4(const char* uniform, const glm::vec4& vec);
    void setMatrix4(const char* uniform, const glm::mat4& mat);
    void setIntArray(const char* uniform, const int* array, int size);
    void setVec2Array(const char* uniform, const glm::vec2* array, int size);
    void setVec3Array(const char* uniform, const glm::vec3* array, int size);
    void setUBO(const char* uniformBlockName, GLuint bindingIndex);
};

class ComputeShader
{
public:

    GLuint ID;

    ComputeShader(const std::string& path);

    void setBool(const char* uniform, bool val);
    void setInt(const char* uniform, int val);
    void setUint(const char* uniform, int val);
    void setFloat(const char* uniform, float val);
    void setVec2(const char* uniform, const glm::vec2& vec);
    void setVec3(const char* uniform, const glm::vec3& vec);
    void setVec4(const char* uniform, const glm::vec4& vec);
    void setMatrix4(const char* uniform, const glm::mat4& mat);
    void setIntArray(const char* uniform, const int* array, int size);
    void setVec2Array(const char* uniform, const glm::vec2* array, int size);
    void setVec3Array(const char* uniform, const glm::vec3* array, int size);
    void setUBO(const char* uniformBlockName, GLuint bindingIndex);

    void bindSSBOToBlock(const SSBO& ssbo, const char* blockName);

    void Activate();
    void Delete();
};