#include "ofApp.h"

/* Helpers */

/* Finds the altitude of the lander(distance between the landerand the terrain)
 * by using ray-based collision detection with the terrain. */
float ofApp::computeAGL() {
	ofVec3f pos = lander->getPosition();
	ofVec3f rayDirection = ofVec3f(0, -1, 0);

	// Create ray from lander towards terrain
	Ray ray = Ray(
		Vector3(pos.x, pos.y, pos.z), 
		Vector3(rayDirection.x, rayDirection.y, rayDirection.z)
	);

	// Detect which node the ray collides with
	TreeNode node;
	bool nodeFound = octree.intersect(ray, octree.root, node);

	if (nodeFound) {
		// Compute distance between lander position and point on terrain
		ofVec3f nodePos = octree.mesh.getVertex(node.points[0]);

		return pos.y - nodePos.y;
	}

	return 0;
}

// Load vertex buffer in preparation for rendering
void ofApp::loadVbo() {
	if (emitter->sys->particles.size() < 1) {
		return;
	}

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter->sys->particles.size(); i++) {
		points.push_back(emitter->sys->particles[i].position);
		sizes.push_back(ofVec3f(emitter->particleRadius));
	}

	// Upload data to the vbo
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;

	// Set up cameras
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	cam.disableMouseInput();

	topCam.setNearClip(.1);
	topCam.setFov(65.5);
	topCam.setPosition(0, 15, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));

	theCam = &cam;

	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	bBackgroundLoaded = backgroundImage.load("images/starfield-plain.jpg");

	// Set up basic lighting
	initLightingAndMaterials();

	// Load terrain
	if (terrain.loadModel("geo/moon-houdini.obj")) {
		terrain.setScaleNormalization(false);
	}
	else {
		cout << "Error: Can't load model: geo/moon-houdini.obj" << endl;
		ofExit(0);
	}

	// Particle shaders & texture

	// Load particle textures and shaders
	//if (!ofLoadImage(particleTexture, "images/dot.png")) {
	//	cout << "Error: Can't load texture file: images/dot.png not found" << endl;
	//	ofExit(0);
	//}

	//ofDisableArbTex();

//#ifdef TARGET_OPENGLES
//	cout << "pass 2" << endl;
//	if (!shader.load("shaders_gles/shader")) {
//		cout << "Error: Can't load shader file: shaders_gles/shader not found" << endl;
//		ofExit(0);
//	}
//#else
//	cout << "pass 3" << endl;
//	if (!shader.load("shaders/shader.vert", "shaders/shader.frag")) {
//		cout << "Error: Can't load shader file: shaders/shader not found" << endl;
//		ofExit(0);
//	}
//#endif

	// Create Octree
	octree.create(terrain.getMesh(0), 20);

	lander = new LunarLander();

	// load lander model
	if (lander->model.loadModel("geo/lander.obj")) {
		lander->model.setScaleNormalization(false);
		lander->model.setScale(.5, .5, .5);
		lander->model.setRotation(0, 0, 1, 0, 0);
		lander->model.setPosition(0, 0, 0);

		bLanderLoaded = true;
	}
	else {
		cout << "Error: Can't load model" << "geo/lander.obj" << endl;
		ofExit(0);
	}

	// Set up forces
	thrustForce = new ThrustForce(ofVec3f(0, 0, 0));
	tanForce = new TangentialForce(ofVec3f(0, 0, 0));
	particleForce = new ThrustForce(ofVec3f(0, -particleThrust, 0));
	turbForce = new TurbulenceForce(
		ofVec3f(-2.0f, -2.0f, -2.0f),
		ofVec3f(2.0f, 2.0f, 2.0f)
	);
	gravityForce = new GravityForce(gravity);

	// Set up particle system
	particleSys = new ParticleSystem();
	emitter = new ParticleEmitter(particleSys);

	emitter->sys->addForce(particleForce);
	emitter->sys->addForce(turbForce);
	emitter->active = true;
	emitter->radius = 0.2f;
	emitter->rate = 20.0f;
	emitter->particleRadius = 0.02f;
	emitter->lifespan = 0.5f;
	emitter->groupSize = 20;
	emitter->particleVelocity = ofVec3f(0, -0.5f, 0);
}

//--------------------------------------------------------------
void ofApp::update(){
	// Apply updates to forces depending on which keys are pressed

	// Apply rotational forces
	tanForce->setTorque(ofVec3f(0, 0, 0));
	if (keymap['a']) {
		tanForce->setTorque(
			tanForce->getTorque() + ofVec3f(0, -torqueMagnitude, 0)
		);
		emitter->start();
	}
	if (keymap['d']) {
		tanForce->setTorque(
			tanForce->getTorque() + ofVec3f(0, torqueMagnitude, 0)
		);
		emitter->start();
	}

	// Apply directional (thruster) forces
	thrustForce->setThrust(ofVec3f(0, 0, 0));
	if (keymap['w']) {
		thrustForce->setThrust(
			thrustForce->getThrust() + ofVec3f(0, thrustMagnitude, 0)
		);
		emitter->start();
	}
	if (keymap['s']) {
		thrustForce->setThrust(
			thrustForce->getThrust() + ofVec3f(0, -thrustMagnitude, 0)
		);
		emitter->start();
	}
	if (keymap[OF_KEY_UP]) {
		thrustForce->setThrust(
			thrustForce->getThrust() + (thrustMagnitude * lander->getForwardUV())
		);
		emitter->start();
	}
	if (keymap[OF_KEY_DOWN]) {
		thrustForce->setThrust(
			thrustForce->getThrust() + (thrustMagnitude * lander->getBackwardUV())
		);
		emitter->start();
	}
	if (keymap[OF_KEY_LEFT]) {
		thrustForce->setThrust(
			thrustForce->getThrust() + (thrustMagnitude * lander->getLeftUV())
		);
		emitter->start();
	}
	if (keymap[OF_KEY_RIGHT]) {
		thrustForce->setThrust(
			thrustForce->getThrust() + (thrustMagnitude * lander->getRightUV())
		);
		emitter->start();
	}

	// Apply forces on the lander
	thrustForce->update(lander);
	tanForce->update(lander);
	turbForce->update(lander);
	gravityForce->update(lander);
	lander->integrate();

	// Reduce fuel if thrusters are being used
	float thrustMagnitude = thrustForce->getThrust().length();
	float tanMagnitude = tanForce->getTorque().length();
	if (thrustMagnitude != 0 || tanMagnitude != 0) {
		float dt = 1.0f / ofGetFrameRate();
		fuel -= dt;
	}

	emitter->position = lander->getPosition();
	emitter->update();

	// Compute lander bounds
	ofVec3f min = lander->model.getSceneMin() + lander->getPosition();
	ofVec3f max = lander->model.getSceneMax() + lander->getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	// Update collision boxes
	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	// Handle lander collision with the terrain
	if (colBoxList.size() >= 5) {
		// Apply impulse to the lander upon collision
		ofVec3f yNormal = ofVec3f(0, 1, 0);
		lander->velocity = (yNormal.dot(-lander->velocity) * yNormal) * 1.25;

		// Check if lander is on the lander area

		cout << lander->velocity.length() << endl;
		// Explode if lander is too fast
		if (lander->velocity.length() >= 2.0f) {
			cout << "explode" << endl;
		}

		// Check if lander successfully landed
		if (lander->velocity.length() < 1.0f) {
			cout << "successful landing" << endl;
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableDepthTest();

	// draw background image
	//
	if (bBackgroundLoaded) {
		ofPushMatrix();
		ofDisableDepthTest();
		ofSetColor(50, 50, 50);
		ofScale(2, 2);
		backgroundImage.draw(-200, -100);
		ofEnableDepthTest();
		ofPopMatrix();
	}

	//loadVbo();

	theCam->begin();

	ofPushMatrix();

	// apply forces


	if (bWireframe) {
		// wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		terrain.drawWireframe();
		
		lander->model.drawWireframe();
	}
	else {
		// shaded mode
		ofEnableLighting();
		terrain.drawFaces();
		lander->model.drawFaces();
	}

	if (bDisplayPoints) {                
		// display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
	}

	// draw shaders
	

	/*particleTexture.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter->sys->particles.size());
	particleTexture.unbind();*/

	// Draw Particle emitter
	emitter->draw();

	// Draw lander and collision boxes
	ofNoFill();
	ofVec3f min = lander->model.getSceneMin() * 0.5 + lander->getPosition();
	ofVec3f max = lander->model.getSceneMax() * 0.5 + lander->getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	ofSetColor(ofColor::white);
	Octree::drawBox(bounds);

	ofSetColor(ofColor::lightBlue);
	for (int i = 0; i < colBoxList.size(); i++) {
		Octree::drawBox(colBoxList[i]);
	}
	

	ofPopMatrix();

	theCam->end();

	// draw screen data
	string str;
	str += "Frame Rate: " + std::to_string(ofGetFrameRate());
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);

	if (showAGL) {
		ofSetColor(ofColor::white);
		ofDrawBitmapString("Altitude (AGL): " + std::to_string(computeAGL()), 5, 15);
	}

	ofSetColor(ofColor::white);
	ofDrawBitmapString("Fuel left: " + std::to_string(fuel), ofGetWindowWidth() - 170, 30);

	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	keymap[key] = true;

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		// Toggle displaying AGL.
		showAGL = !showAGL;
		break;
	case 'P':
	case 'p':
		break;
	case 'r':
		cam.reset();
		break;
	case 'g':
		savePicture();
		break;
	case 't':
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'm':
		toggleWireframeMode();
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F3:
		theCam = &topCam;
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	// Reset thrust-force in case user stopped pressing a movement key
	keymap[key] = false;

	// Stop emitter in case user stopped pressing a movement key
	emitter->stop();

	switch (key) {
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ .7f, .7f, .7f, 1.0f };

	static float position[] =
	{ 20.0, 20.0, 20.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	//	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	//	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	//	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

void ofApp::exit() {
	free(thrustForce);
	free(tanForce);
	free(particleForce);
	free(turbForce);
	free(emitter);
	free(particleSys);
	free(lander);
}