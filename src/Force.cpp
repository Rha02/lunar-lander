#include "Force.h"

ThrustForce::ThrustForce(const ofVec3f& thrust) {
	this->thrust = thrust;
}

ofVec3f ThrustForce::getThrust() {
	return this->thrust;
}

void ThrustForce::setThrust(const ofVec3f& thrust) {
	this->thrust = thrust;
}

void ThrustForce::update(PhysicsObject* obj) {
	obj->forces += thrust;
}

TangentialForce::TangentialForce(const ofVec3f& torque) {
	this->torque = torque;
}

ofVec3f TangentialForce::getTorque() {
	return this->torque;
}

void TangentialForce::setTorque(const ofVec3f& torque) {
	this->torque = torque;
}

void TangentialForce::update(PhysicsObject* obj) {
	// Torque Formula
	// t = F * r
	obj->tangentialForces += torque / (obj->radius);
}

TurbulenceForce::TurbulenceForce(const ofVec3f& tmin, const ofVec3f& tmax) {
	this->tmax = tmax;
	this->tmin = tmin;
}

void TurbulenceForce::setTurbulence(const ofVec3f& tmin, const ofVec3f& tmax) {
	this->tmin = tmin;
	this->tmax = tmax;
}

void TurbulenceForce::update(PhysicsObject* obj) {
	obj->forces += ofVec3f(
		ofRandom(tmin.x, tmax.x),
		ofRandom(tmin.y, tmax.y),
		ofRandom(tmin.z, tmax.z)
	);
}

GravityForce::GravityForce(float gravity) {
	this->gravity = gravity;
}

void GravityForce::setGravity(float gravity) {
	this->gravity = gravity;
}

void GravityForce::update(PhysicsObject* obj) {
	float gForce = obj->mass * -gravity;

	obj->forces += ofVec3f(0, gForce, 0);
}

ImpulseRadialForce::ImpulseRadialForce(float magnitude) {
	this->magnitude = magnitude;
}

void ImpulseRadialForce::setMagnitude(float magnitude) {
	this->magnitude = magnitude;
}

void ImpulseRadialForce::update(PhysicsObject* obj) {
	ofVec3f dir = ofVec3f(
		ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1)
	);
	obj->forces += dir.normalize() * (magnitude * ofRandom(0.8f, 1.0f));
}