#pragma once

#include "ParticleSystem.h"

typedef enum { DiskEmitter, RadialEmitter } EmitterType;

class ParticleEmitter {
private:
	void spawnParticle(float);
public:
	ParticleEmitter(ParticleSystem*);

	ParticleSystem* sys;
	EmitterType type;

	ofVec3f position;
	ofVec3f particleVelocity;
	float rate = 1.0f;
	float lifespan = 3.0f;
	float mass = 100.0f;
	float particleRadius = 0.1f;
	float radius = 0.5f;
	int groupSize = 20;
	bool active = false;
	int lastSpawned = 0;
	bool oneShot = false;
	bool fired = false;

	void start();
	void stop();
	void draw();
	void update();
};