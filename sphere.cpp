#include "sphere.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

Sphere::Sphere()
{
    isInited = false;
    sphere_vao = 0;
    sphere_vboVertex = 0;
    sphere_vboIndex = 0;

    sectorCount = 36;
    stackCount = 18;
}

Sphere::~Sphere()
{

}

void Sphere::init(GLuint vertexPositionID, float radius)
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<GLuint> lineIndices;
    std::vector<GLuint> normals;
    std::vector<GLuint> texCoords;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * glm::pi<double>() / sectorCount;
    float stackStep = glm::pi<double>() / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
    stackAngle = glm::pi<double>() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
    xy = radius * cosf(stackAngle);             // r * cos(u)
    z = radius * sinf(stackAngle);              // r * sin(u)

    // add (sectorCount+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for(int j = 0; j <= sectorCount; ++j)
        {
        sectorAngle = j * sectorStep;           // starting from 0 to 2pi

        // vertex position (x, y, z)
        x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
        y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        // normalized vertex normal (nx, ny, nz)
        nx = x * lengthInv;
        ny = y * lengthInv;
        nz = z * lengthInv;
        normals.push_back(nx);
        normals.push_back(ny);
        normals.push_back(nz);

        // vertex tex coord (s, t) range between [0, 1]
        s = (float)j / sectorCount;
        t = (float)i / stackCount;
        texCoords.push_back(s);
        texCoords.push_back(t);
        }    
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
    k1 = i * (sectorCount + 1);     // beginning of current stack
    k2 = k1 + sectorCount + 1;      // beginning of next stack

    for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
        // 2 triangles per sector excluding first and last stacks
        // k1 => k2 => k1+1
        if(i != 0)
            {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);
            }

        // k1+1 => k2 => k2+1
        if(i != (stackCount-1))
            {
            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
            }

        // store indices for lines
        // vertical lines for all stacks, k1 => k2
        lineIndices.push_back(k1);
        lineIndices.push_back(k2);
        if(i != 0)  // horizontal lines except 1st stack, k1 => k+1
            {
            lineIndices.push_back(k1);
            lineIndices.push_back(k1 + 1);
            }
        }
    }
    glGenVertexArrays(1, &sphere_vao);
    glBindVertexArray(sphere_vao);

    glGenBuffers(1, &sphere_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vboVertex);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(vertexPositionID, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray (vertexPositionID);

    glGenBuffers(1, &sphere_vboIndex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_vboIndex);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    numsToDraw = indices.size();

    std::ofstream sphere_vertex_log_file;
    sphere_vertex_log_file.open("sphere_v.log");
	for (int k = 0;k<=vertices.size();k++){
        sphere_vertex_log_file << " Vertices[" << k << "]: " << vertices[k] << std::endl;
    }
    sphere_vertex_log_file.close();

    std::ofstream sphere_index_log_file;
    sphere_index_log_file.open("sphere_i.log");
	for (int k = 0;k<=indices.size();k++){ 
        sphere_index_log_file << " Indices[" << k << "]: " << indices[k] << std::endl;
    }
    sphere_index_log_file.close();

    isInited = true;
}

void Sphere::cleanup()
{
    if (!isInited) {
        return;
    }
    if(sphere_vboVertex) {
        glDeleteBuffers(1, &sphere_vboVertex);
    }
    if(sphere_vboIndex) {
        glDeleteBuffers(1, &sphere_vboIndex);
    }
    if (sphere_vao) {
        glDeleteVertexArrays(1, &sphere_vao);
    }

    isInited = false;
    sphere_vao = 0;
    sphere_vboVertex = 0;
    sphere_vboIndex = 0;
}

void Sphere::draw()
{
    if (!isInited) {
        std::cout << "please call init() before draw()" << std::endl;
    }

    // draw sphere
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glBindVertexArray(sphere_vao);
    glDrawElements(GL_TRIANGLES, numsToDraw, GL_UNSIGNED_INT, NULL);
}