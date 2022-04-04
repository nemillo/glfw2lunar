#ifndef LINE_H
#define LINE_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class Line
{
public:
    Line();
    ~Line();
    void init(GLuint vertexPositionID, glm::vec3 a, glm::vec3 b);
    void cleanup();
    void draw();

private:
    bool isInited;
    GLuint line_vao, line_vboVertex, line_vboIndex;
    int numsToDraw;
};

#endif // LINE_H