#include "line.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

Line::Line()
{
    isInited = false;
    line_vao = 0;
    line_vboVertex = 0;
    line_vboIndex = 0;

}

Line::~Line()
{

}

void Line::init(GLuint vertexPositionID, glm::vec3 a, glm::vec3 b)
{
    
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
       
    vertices.push_back(a.x);
    vertices.push_back(a.y);
    vertices.push_back(a.z);

    vertices.push_back(b.x);
    vertices.push_back(b.y);
    vertices.push_back(b.z);
           
    indices.push_back(0);
    indices.push_back(1);
    
    glGenVertexArrays(1, &line_vao);
    glBindVertexArray(line_vao);

    glGenBuffers(1, &line_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, line_vboVertex);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(vertexPositionID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (vertexPositionID);

    glGenBuffers(1, &line_vboIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    numsToDraw = indices.size();
    
    isInited = true;
}

void Line::cleanup()
{
    if (!isInited) {
        return;
    }
    if(line_vboVertex) {
        glDeleteBuffers(1, &line_vboVertex);
    }
    if(line_vboIndex) {
        glDeleteBuffers(1, &line_vboIndex);
    }
    if (line_vao) {
        glDeleteVertexArrays(1, &line_vao);
    }

    isInited = false;
    line_vao = 0;
    line_vboVertex = 0;
    line_vboIndex = 0;
}

void Line::draw()
{
    if (!isInited) {
        std::cout << "please call init() before draw()" << std::endl;
    }

    // draw line
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBindVertexArray(line_vao);
    glDrawElements(GL_LINES, numsToDraw, GL_UNSIGNED_INT, NULL);
}