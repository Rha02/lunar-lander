#include "Particle.h"

// Set up particle
Particle::Particle() {
	velocity.set(0, 0, 0);
	position.set(0, 0, 0);
	forces.set(0, 0, 0);
	lifespan = 5;
	birthtime = 0;
	radius = .1;
	damping = .99;
	mass = 1;
	color.set(245, 158, 11);
}

void Particle::draw() {
	// Compute a value by how much to make the color darker
	float darkeningFactor = ofMap(age(), 0, lifespan, 1.0f, 0.25f);

	// Apply darkening factor
	ofVec3f darkenedColor = color * darkeningFactor;

	ofSetColor(darkenedColor.x, darkenedColor.y, darkenedColor.z);

	ofDrawSphere(position, radius);
}

void Particle::integrate() {
	float dt = 1.0 / ofGetFrameRate();

	if (isnan(dt) || isinf(dt)) {
		return;
	}

	// Update position
	position += velocity * dt;

	// Apply forces to update velocity
	ofVec3f acceleration = forces / mass;
	velocity += acceleration * dt;

	velocity *= damping;

	forces.set(0, 0, 0);
}

// Get age of particle in seconds
float Particle::age() {
	return (ofGetElapsedTimeMillis() - birthtime) / 1000.0;
}