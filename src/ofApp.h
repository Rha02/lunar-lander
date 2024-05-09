#pragma once

#include "ofMain.h"
#include "Force.h"
#include "LunarLander.h"
#include "ParticleEmitter.h"
#include "Octree.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

class ofApp : public ofBaseApp{
private:
	float computeAGL();
	bool showAGL = true;

	Octree octree;
	TreeNode selectedNode;
	Box boundingBox, landerBounds;

public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void drawAxis(ofVec3f);
	void initLightingAndMaterials();
	void savePicture();
	void toggleWireframeMode();
	void togglePointsDisplay();
		
	ofEasyCam cam;
	LunarLander* lander;
	ofxAssimpModelLoader terrain;
	ofLight light;
	ofImage backgroundImage;
	ofCamera* theCam = NULL;
	ofCamera topCam;

	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bWireframe;
	bool bDisplayPoints;

	bool bBackgroundLoaded = false;
	bool bLanderLoaded = false;

	// Thrust force
	ThrustForce* thrustForce;
	float thrustMagnitude = 200.0f;

	// Tangential force
	TangentialForce* tanForce;
	float torqueMagnitude = 10000.0f;

	// Turbulence force
	TurbulenceForce* turbForce;

	// Gravity force
	GravityForce* gravityForce;
	float gravity = 0.164f;

	// Particle force
	ThrustForce* particleForce;
	float particleThrust = 20.0f;

	// Particles
	ParticleEmitter* emitter;
	ParticleSystem* particleSys;
};
