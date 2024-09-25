#include <OpenGL/UBO.h>
#include <cstring>  // For memcpy

UBO::UBO(GLuint bindIndex, GLsizeiptr size) : bindingIndex(bindIndex)
{
    glGenBuffers(1, &ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ID);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

void UBO::Update(void* data, GLsizeiptr size)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void UBO::Bind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

void UBO::Unbind()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::Delete()
{
    glDeleteBuffers(1, &ID);
}
