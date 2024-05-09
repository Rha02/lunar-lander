#pragma once

#include "ofMain.h"

class PhysicsObject {
public:
	ofVec3f position;
	ofVec3f velocity = glm::vec3(0, 0, 0);
	float rotation = 0;
	float angularVelocity = 0;
	float damping = 0.99f;
	float mass = 1.0f;
	float radius = 1.0f;
	ofVec3f forces;
	ofVec3f tangentialForces;
	virtual void integrate() = 0;
};