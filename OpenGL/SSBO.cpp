#include <OpenGL/SSBO.h>

SSBO::SSBO(void* data, GLsizeiptr size, GLuint bindIndex) : bindingIndex(bindIndex)
{
	glGenBuffers(1, &ID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::Bind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
}

void SSBO::Unbind()
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::Delete()
{
    glDeleteBuffers(1, &ID);
}
