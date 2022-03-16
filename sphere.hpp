#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
#include <glm/glm.hpp>

class Sphere
{
public:
    Sphere();
    ~Sphere();
    void init(GLuint vertexPositionID, float radius);
    void cleanup();
    void draw();
    glm::vec3 getPosition() const { return position; }
	glm::vec3 getVelocity() const { return velocity; }
	glm::vec3 getAcceleration() const { return acceleration; }

	void setPosition(const glm::vec3& a) { position = a; }
	void setVelocity(const glm::vec3& a) { velocity = a; }
	void setAcceleration(const glm::vec3& a) { acceleration = a; }

	float getMass() const { return mass; }
	void setMass(const float a) { mass = a; }

private:
    int sectorCount, stackCount;
    bool isInited;
    GLuint sphere_vao, sphere_vboVertex, sphere_vboIndex;
    int numsToDraw;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;
    

};

#endif // SPHERE_H