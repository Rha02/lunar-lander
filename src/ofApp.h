#pragma once

#include "ofMain.h"
#include "Force.h"
#include "LunarLander.h"
#include "ParticleEmitter.h"
#include "Octree.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

enum GameState {
	PREGAME, INGAME, ENDGAME
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

	glm::vec3 getMousePointOnPlane(glm::vec3 p, glm::vec3 n);
		
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

	GameState gamestate = PREGAME;
	ofTrueTypeFont textDisplay;

	// Thrust force
	ThrustForce* thrustForce;
	float thrustMagnitude = 150.0f;

	// Tangential force
	TangentialForce* tanForce;
	float torqueMagnitude = 8000.0f;

	// Turbulence force
	TurbulenceForce* turbForce;

	// Gravity force
	GravityForce* gravityForce;
	float gravity = 1.64f;

	// Particle forces
	ThrustForce* particleForce;
	float particleThrust = 20.0f;

	ImpulseRadialForce* explosionForce;
	float explosionMagnitude = 600.0f;

	// Particles
	ParticleEmitter* emitter;
	ParticleSystem* particleSys;

	ParticleEmitter* explosionEmitter;
	ParticleSystem* explosionParticleSys;

	// LEM fuel
	float fuel = 120;

	int score = 0;

	// Particle System Shades
	ofTexture particleTexture;
	ofVbo vbo;
	ofShader shader;
	void loadVbo();

	map<int, bool> keymap;
};
