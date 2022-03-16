#ifndef PLANE_H
#define PLANE_H

#include <GL/glew.h>

class Plane
{
public:
    Plane();
    ~Plane();
    void init(GLuint vertexPositionID, float z0);
    void cleanup();
    void draw();

private:
    int divsx, divsy;
    bool isInited;
    GLuint plane_vao, plane_vboVertex, plane_vboIndex;
    int numsToDraw;
};

#endif // SPHERE_H