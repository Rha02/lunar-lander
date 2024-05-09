#include "LunarLander.h"

LunarLander::LunarLander() {
	position = model.getPosition();
	mass = 10.0f;
	radius = 3.0f;
	rotation = model.getRotationAngle(1);
}

ofVec3f LunarLander::getPosition() {
	return model.getPosition();
}

void LunarLander::setPosition(const ofVec3f& pos) {
	model.setPosition(pos.x, pos.y, pos.z);
	position = pos;
}

void LunarLander::integrate() {
	float dt = 1.0f / ofGetFrameRate();

	if (isnan(dt) || isinf(dt)) {
		return;
	}

	// apply linear motion forces
	glm::vec3 pos = position + velocity * dt;
	setPosition(pos);

	// Force formula, where a is linear acceleration
	// F = m * a
	glm::vec3 acceleration = forces / mass;

	velocity += acceleration * dt;
	velocity *= damping;

	// apply rotational motion forces
	float rot = rotation + angularVelocity * dt;
	setRotationAngle(rot);

	// tangential force formula, where a is angular velocity
	// F = m * r * a
	glm::vec3 angularAcceleration = tangentialForces / (mass * radius);

	float direction = 1;
	if (angularAcceleration.y > 0) {
		direction = -1;
	}

	// Update angularVelocity
	angularVelocity += direction * glm::length(angularAcceleration) * dt;
	angularVelocity *= damping;

	// reset forces
	forces.set(0, 0, 0);
	tangentialForces.set(0, 0, 0);
}

float LunarLander::getRotationAngle() {
	return rotation;
}

void LunarLander::setRotationAngle(float a) {
	model.setRotation(
		1, a, 0, 1, 0
	);
	rotation = a;
}

ofVec3f LunarLander::getForwardUV() {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::vec3 directionUV = glm::normalize(rotationMatrix * glm::vec4(0, 0, 1, 1));
	return directionUV;
}

ofVec3f LunarLander::getBackwardUV() {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::vec3 directionUV = glm::normalize(rotationMatrix * glm::vec4(0, 0, -1, 1));
	return directionUV;
}

ofVec3f LunarLander::getLeftUV() {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::vec3 directionUV = glm::normalize(rotationMatrix * glm::vec4(1, 0, 0, 1));
	return directionUV;
}

ofVec3f LunarLander::getRightUV() {
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::vec3 directionUV = glm::normalize(rotationMatrix * glm::vec4(-1, 0, 0, 1));
	return directionUV;
}