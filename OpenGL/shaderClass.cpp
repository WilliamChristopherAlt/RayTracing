#include <OpenGL/shaderClass.h>

// Helper function to check for shader compilation/linking errors.
void compileErrors(GLuint shader, const char* type)
{
    GLint hasCompiled;
    char infoLog[1024];
    if (type != std::string("PROGRAM"))
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
}

Shader::Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath)
{
    std::string vertexCode = getFileContents(vertexFilePath);
    std::string fragmentCode = getFileContents(fragmentFilePath);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Vertex Shader Object
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    compileErrors(vertexShader, "VERTEX");

    // Fragment Shader Object
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT");

    // Shader Program Object
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    compileErrors(ID, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
    glUseProgram(ID);
}

void Shader::Delete()
{
    glDeleteProgram(ID);
}

void Shader::setBool(const char* uniform, bool val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1i(uniformLoc, val);
}

void Shader::setInt(const char* uniform, int val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1i(uniformLoc, val);
}

void Shader::setUint(const char* uniform, int val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1ui(uniformLoc, val);
}

void Shader::setFloat(const char* uniform, float val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1f(uniformLoc, val);
}

void Shader::setVec2(const char* uniform, const glm::vec2& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform2fv(uniformLoc, 1, glm::value_ptr(vec));
}

void Shader::setVec3(const char* uniform, const glm::vec3& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform3fv(uniformLoc, 1, glm::value_ptr(vec));
}

void Shader::setVec4(const char* uniform, const glm::vec4& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform4fv(uniformLoc, 1, glm::value_ptr(vec));
}

void Shader::setMatrix4(const char* uniform, const glm::mat4& mat)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setIntArray(const char* uniform, const int* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform1iv(uniformLoc, size, array);
}

void Shader::setVec2Array(const char* uniform, const glm::vec2* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform2fv(uniformLoc, size, glm::value_ptr(array[0]));
}

void Shader::setVec3Array(const char* uniform, const glm::vec3* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform3fv(uniformLoc, size, glm::value_ptr(array[0]));
}

void Shader::setUBO(const char* uniformBlockName, GLuint bindingIndex)
{
    GLuint blockIndex = glGetUniformBlockIndex(ID, uniformBlockName);
    glUniformBlockBinding(ID, blockIndex, bindingIndex);
}

ComputeShader::ComputeShader(const std::string& path)
{
    std::string codeStr = getFileContents(path);
    const char* code = codeStr.c_str();;

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &code, NULL);
    glCompileShader(computeShader);
    compileErrors(computeShader, "COMPUTE");

    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);
    compileErrors(ID, "PROGRAM");

    // glDeleteShader(computeShader);
}

void ComputeShader::setBool(const char* uniform, bool val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1i(uniformLoc, val);
}

void ComputeShader::setInt(const char* uniform, int val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1i(uniformLoc, val);
}

void ComputeShader::setUint(const char* uniform, int val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1ui(uniformLoc, val);
}

void ComputeShader::setFloat(const char* uniform, float val)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform1f(uniformLoc, val);
}

void ComputeShader::setVec2(const char* uniform, const glm::vec2& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform2fv(uniformLoc, 1, glm::value_ptr(vec));
}

void ComputeShader::setVec3(const char* uniform, const glm::vec3& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform3fv(uniformLoc, 1, glm::value_ptr(vec));
}

void ComputeShader::setVec4(const char* uniform, const glm::vec4& vec)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniform4fv(uniformLoc, 1, glm::value_ptr(vec));
}

void ComputeShader::setMatrix4(const char* uniform, const glm::mat4& mat)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUseProgram(ID);
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

void ComputeShader::setIntArray(const char* uniform, const int* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform1iv(uniformLoc, size, array);
}

void ComputeShader::setVec2Array(const char* uniform, const glm::vec2* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform2fv(uniformLoc, size, glm::value_ptr(array[0]));
}

void ComputeShader::setVec3Array(const char* uniform, const glm::vec3* array, int size)
{
    GLuint uniformLoc = glGetUniformLocation(ID, uniform);
    glUniform3fv(uniformLoc, size, glm::value_ptr(array[0]));
}


void ComputeShader::Activate()
{
    glUseProgram(ID);
}

void ComputeShader::Delete()
{
    glDeleteProgram(ID);
}

void ComputeShader::bindSSBOToBlock(const SSBO& ssbo, const char* blockName)
{
    glUseProgram(ID);
    glUniformBlockBinding(ID, glGetUniformBlockIndex(ID, blockName), ssbo.bindingIndex);
}