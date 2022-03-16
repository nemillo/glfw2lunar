#include "plane.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

Plane::Plane()
{
    isInited = false;
    plane_vao = 0;
    plane_vboVertex = 0;
    plane_vboIndex = 0;

    divsx = 2;
    divsy = 2;
}

Plane::~Plane()
{

}

void Plane::init(GLuint vertexPositionID, float z0)
{
    int i, j,k;
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    
    for(i = -divsx; i <= divsx; ++i) {
       
       for(j = -divsy; j <= divsy; ++j) {
           double x = j;
           double y = i;
           double z = z0;
           vertices.push_back(x);
           vertices.push_back(y);
           vertices.push_back(z);
           
        }
    }

    for (int r=0; r< 2*divsx; r++)
    {
    //Set idx to point at first vertex of row r
        int idx=r*(2*divsx+1);

        for (int c=0; c< 2*divsy; c++)
        {        
          //Bottom triangle of the quad
          indices.push_back(idx);
          indices.push_back(idx+1);
          indices.push_back(idx+2*divsx+1);
          //Top triangle of the quad
          indices.push_back(idx+1);
          indices.push_back(idx+2*divsx+2);
          indices.push_back(idx+2*divsx+1);
          //Move one vertex to the right
          idx++;
        }
    }

    glGenVertexArrays(1, &plane_vao);
    glBindVertexArray(plane_vao);

    glGenBuffers(1, &plane_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vboVertex);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(vertexPositionID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (vertexPositionID);

    glGenBuffers(1, &plane_vboIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    numsToDraw = indices.size();
    
    std::ofstream plane_vertex_log_file;
    plane_vertex_log_file.open("plane_v.log");
	int v_number = 0;
    plane_vertex_log_file << "vertices.size(): " << vertices.size() << std::endl;
    for (k = 0;k<vertices.size();k=k+3){
        plane_vertex_log_file << " Vertex[" << v_number << "]: " << vertices[k] << "   " << vertices[k+1] << "   " << vertices[k+2] << std::endl ;
        v_number++;
    }
    plane_vertex_log_file.close();

    std::ofstream plane_index_log_file;
    plane_index_log_file.open("plane_i.log");
    plane_index_log_file << "indices.size(): " << indices.size() << std::endl;
	for (k = 0;k<indices.size();++k){ 
        plane_index_log_file << " Indices[" << k << "]: " << indices[k] << std::endl;
    }
    plane_index_log_file.close();

    isInited = true;
}

void Plane::cleanup()
{
    if (!isInited) {
        return;
    }
    if(plane_vboVertex) {
        glDeleteBuffers(1, &plane_vboVertex);
    }
    if(plane_vboIndex) {
        glDeleteBuffers(1, &plane_vboIndex);
    }
    if (plane_vao) {
        glDeleteVertexArrays(1, &plane_vao);
    }

    isInited = false;
    plane_vao = 0;
    plane_vboVertex = 0;
    plane_vboIndex = 0;
}

void Plane::draw()
{
    if (!isInited) {
        std::cout << "please call init() before draw()" << std::endl;
    }

    // draw plane
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBindVertexArray(plane_vao);
    glDrawElements(GL_TRIANGLES, numsToDraw, GL_UNSIGNED_INT, NULL);
}