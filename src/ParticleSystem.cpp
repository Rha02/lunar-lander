#include "ParticleSystem.h"

void add(const Particle&);
void addForce(Force*);
void remove(int);
void update();
void setLifespan(float);
void reset();
void draw();

// add a particle to the system
void ParticleSystem::add(const Particle& p) {
	particles.push_back(p);
}

// add a force applied on the particles
void ParticleSystem::addForce(Force* f) {
	forces.push_back(f);
}

// remove a particle by index
void ParticleSystem::remove(int i) {
	particles.erase(particles.begin() + i);
}

void ParticleSystem::update() {
	// If no particles, don't do anything
	if (particles.size() == 0) {
		return;
	}

	// Remove any particles that have exceeded their lifetime
	vector<Particle>::iterator iter = particles.begin();
	vector<Particle>::iterator tmp;
	while (iter != particles.end()) {
		if (iter->age() > iter->lifespan) {
			tmp = particles.erase(iter);
			iter = tmp;
		}
		else {
			iter++;
		}
	}

	// apply forces on each particle
	for (int i = 0; i < particles.size(); i++) {
		for (int j = 0; j < forces.size(); j++) {
			forces[j]->update(&particles[i]);
		}
	}

	// integrate each particle
	for (int i = 0; i < particles.size(); i++) {
		particles[i].integrate();
	}
}

void ParticleSystem::setLifespan(float ls) {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].lifespan = ls;
	}
}

void ParticleSystem::draw() {
	// draw each particle
	for (int i = 0; i < particles.size(); i++) {
		particles[i].draw();
	}
}