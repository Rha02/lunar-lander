#pragma once

#include "PhysicsObject.h"

class Particle : public PhysicsObject {
public:
	Particle();
	float   lifespan;
	float   birthtime;
	void    integrate();
	void    draw();
	float   age();
	ofVec3f color;
};