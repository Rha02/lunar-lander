#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(ParticleSystem* s) {
	position.set(0, 0, 0);
	particleVelocity.set(0, -0.5, 0);
	sys = s;
	type = DiskEmitter;
}

void ParticleEmitter::start() {
	active = true;
}

void ParticleEmitter::stop() {
	active = false;
}

void ParticleEmitter::draw() {
	sys->draw();
}

void ParticleEmitter::update() {
	float time = ofGetElapsedTimeMillis();

	// Check if emitter is active and that the period of time for next particle to spawn has passed.
	if (active && ((time - lastSpawned) > (1000.0f / rate))) {
		// Spawn a group of particles
		for (int i = 0; i < groupSize; i++) {
			spawnParticle(time);
		}

		lastSpawned = time;

		if (oneShot) {
			active = false;
			stop();
		}
	}

	sys->update();
}

void ParticleEmitter::spawnParticle(float time) {
	Particle p;

	if (type == DiskEmitter) {
		ofVec3f pos = ofVec3f(
			ofRandom(position.x - radius, position.x + radius),
			ofRandom(position.y + 0.20, position.y + 0.25),
			ofRandom(position.z - radius, position.z + radius)
		);
		p.position.set(pos);
		p.velocity = particleVelocity;
	}
	else if (type == RadialEmitter) {
		ofVec3f dir = ofVec3f(
			ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1)
		);
		p.velocity = dir.normalize() * particleVelocity.length();
		p.position.set(position);
	}
	else {
		p.velocity = particleVelocity;
		p.position.set(position);
	}

	p.lifespan = lifespan;
	p.birthtime = time;
	p.radius = particleRadius;

	sys->add(p);
}