#include <OpenGL/VBO.h>

VBO::VBO(std::vector<Vertex>& vertices)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void VBO::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
    glDeleteBuffers(1, &ID);
}

VBOSimple::VBOSimple(GLfloat* vertices, GLsizeiptr size)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBOSimple::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBOSimple::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBOSimple::Delete()
{
    glDeleteBuffers(1, &ID);
}