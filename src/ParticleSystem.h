#pragma once

#include "ofMain.h"
#include "Particle.h"
#include "Force.h"

class ParticleSystem {
public:
	vector<Particle> particles;
	vector<Force*> forces;

	void add(const Particle&);
	void addForce(Force*);
	void remove(int);
	void update();
	void setLifespan(float);
	void draw();
};