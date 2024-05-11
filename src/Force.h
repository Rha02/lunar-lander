#pragma once
#include "ofMain.h"
#include "PhysicsObject.h"

class Force {
public:
	virtual void update(PhysicsObject*) = 0;
	bool applyOnce = false;
	bool applied = false;
};

// ThrustForce is a force used for linear motion
class ThrustForce : public Force {
private:
	ofVec3f thrust;
public:
	ThrustForce(const ofVec3f& thrust);
	ofVec3f getThrust();
	void setThrust(const ofVec3f&);
	void update(PhysicsObject*);
};

// TangentialForce is a force used for rotational motion
class TangentialForce : public Force {
private:
	ofVec3f torque;
public:
	TangentialForce(const ofVec3f& torque);
	ofVec3f getTorque();
	void setTorque(const ofVec3f& torque);
	void update(PhysicsObject*);
};

class TurbulenceForce : public Force {
private:
	ofVec3f tmin, tmax;
public:
	TurbulenceForce(const ofVec3f& tmin, const ofVec3f& tmax);
	void setTurbulence(const ofVec3f& tmin, const ofVec3f& tmax);
	void update(PhysicsObject*);
};

class GravityForce : public Force {
private:
	float gravity;
public:
	GravityForce(float gravity);
	void setGravity(float gravity);
	void update(PhysicsObject*);
};

class ImpulseRadialForce : public Force {
private:
	float magnitude;

public:
	ImpulseRadialForce(float magnitude);
	void setMagnitude(float magnitude);
	void update(PhysicsObject*);
};