#pragma once
#include "ofMain.h"
#include "PhysicsObject.h"
#include "ofxAssimpModelLoader.h"

class LunarLander : public PhysicsObject {
public:
	LunarLander();
	ofxAssimpModelLoader model;
	void integrate();
	ofVec3f getPosition();
	void setPosition(const ofVec3f&);
	float getRotationAngle();
	void setRotationAngle(float);
	ofVec3f getForwardUV();
	ofVec3f getBackwardUV();
	ofVec3f getLeftUV();
	ofVec3f getRightUV();
};