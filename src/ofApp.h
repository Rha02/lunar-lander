#pragma once

#include "ofMain.h"
#include "Force.h"
#include "LunarLander.h"
#include "ParticleEmitter.h"
#include "Octree.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>
#include "ofxGui.h"

enum GameState {
	PREGAME, INGAME, ENDGAME
};

enum GameEnv {
	MOON, DESERT
};

class ofApp : public ofBaseApp{
private:
	float computeAGL();
	bool showAGL = true;

	Octree octree;
	TreeNode selectedNode;
	Box boundingBox, landerBounds;

	bool bLanderSelected = false;
	glm::vec3 mouseDownPos, mouseLastPos;
	bool bInDrag = false;

	vector<Box> colBoxList;

	void setupLander();

	GameEnv gameEnv = DESERT;
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
	void togglePointsDisplay();

	glm::vec3 getMousePointOnPlane(glm::vec3 p, glm::vec3 n);

	LunarLander* lander;
	ofxAssimpModelLoader terrain;
	ofLight light;
	ofImage backgroundImage;

	// Set up cameras
	ofCamera* theCam = NULL;
	ofCamera topCam;
	ofEasyCam freeCam;
	ofEasyCam trackingCam;
	ofEasyCam onboardCam;


	bool bAltKeyDown;
	bool bCtrlKeyDown;
	bool bDisplayPoints;

	bool bBackgroundLoaded = false;

	GameState gamestate = PREGAME;
	ofTrueTypeFont textDisplay;

	// Thrust force
	ThrustForce* thrustForce;
	float thrustMagnitude = 90.0f;

	// Tangential force
	TangentialForce* tanForce;
	float torqueMagnitude = 6000.0f;

	// Turbulence force
	TurbulenceForce* turbForce;

	// Gravity force
	GravityForce* gravityForce;
	float gravity = 1.64f;

	// Particle forces
	ThrustForce* particleForce;
	float particleThrust = 35.0f;

	ImpulseRadialForce* explosionForce;
	float explosionMagnitude = 1400.0f;

	// Particles
	ParticleEmitter* emitter;
	ParticleSystem* particleSys;

	ParticleEmitter* explosionEmitter;
	ParticleSystem* explosionParticleSys;

	// LEM fuel
	float fuel = 120;

	int score = 0;

	ofVec3f landingArea1 = ofVec3f(30, 0.5, -30); // Flat area
	ofVec3f landingArea2 = ofVec3f(-132, 21.5, 36.3); // Mountain area
	ofVec3f landingArea3 = ofVec3f(-25, 10, 90.7); // Inclined area

	// Lighting
	ofLight ambientLight;

	ofLight landingArea1Light;
	ofLight landingArea2Light;
	ofLight landingArea3Light;

	ofLight landerLight;

	bool area1Landed = false;
	bool area2Landed = false;
	bool area3Landed = false;
	bool shipExploded = false;

	// Particle System Shades
	ofTexture particleTexture;
	ofVbo vbo;
	ofShader shader;
	void loadVbo();

	map<int, bool> keymap;
};
