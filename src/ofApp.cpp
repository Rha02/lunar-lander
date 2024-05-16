#include "ofApp.h"

/* Helpers */
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	glm::vec3 origin = theCam->getPosition();
	glm::vec3 camAxis = theCam->getZAxis();
	glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}

void ofApp::setupLander() {
	lander = new LunarLander();

	string modelPath = "geo/lander.obj";

	if (gameEnv == DESERT) {
		modelPath = "geo/ufo_lander.fbx";
	}

	// load lander model
	if (lander->model.loadModel(modelPath)) {
		lander->model.setScaleNormalization(false);
		lander->model.setScale(.5, .5, .5);
		lander->model.setRotation(0, 0, 1, 0, 0);
		lander->setPosition(ofVec3f(0, 15.0f, 0));
	}
	else {
		cout << "Error: Can't load model " << modelPath << endl;
		ofExit(0);
	}
}

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
	if (emitter->sys->particles.size() < 1 && explosionEmitter->sys->particles.size() < 1) {
		return;
	}

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter->sys->particles.size(); i++) {
		points.push_back(emitter->sys->particles[i].position);
		sizes.push_back(ofVec3f(emitter->particleRadius));
	}

	for (int i = 0; i < explosionEmitter->sys->particles.size(); i++) {
		points.push_back(explosionEmitter->sys->particles[i].position);
		sizes.push_back(ofVec3f(explosionEmitter->particleRadius));
	}

	// Upload data to the vbo
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

//--------------------------------------------------------------
void ofApp::setup(){
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;

	ofSetVerticalSync(true);
	ofEnableSmoothing();
	ofEnableDepthTest();

	bBackgroundLoaded = backgroundImage.load("images/starfield-plain.jpg");

	// Load custom font
	textDisplay.load("fonts/LucidaConsole.ttf", 16);

	// Set up basic lighting
	initLightingAndMaterials();

	// Load terrain
	string terrainPath = "geo/moon-houdini.obj";
	if (gameEnv == DESERT) {
		terrainPath = "geo/terrain.fbx";

		// Change landing areas for the Desert terrain
		landingArea1 = ofVec3f(42.5, -0.9, 15.5);
		landingArea2 = ofVec3f(28.2, 6.0, 81.7);
		landingArea3 = ofVec3f(-106.7, 34.5, 29.7);
	}
	if (terrain.loadModel(terrainPath)) {
		terrain.setScaleNormalization(false);
	}
	else {
		cout << "Error: Can't load model " << terrainPath << endl;
		ofExit(0);
	}

	// Particle shaders & texture

	ofDisableArbTex();

	// Load particle textures and shaders
	if (!ofLoadImage(particleTexture, "images/dot.png")) {
		cout << "Error: Can't load texture file: images/dot.png not found" << endl;
		ofExit(0);
	}

#ifdef TARGET_OPENGLES
	if (!shader.load("shaders_gles/shader")) {
		cout << "Error: Can't load shader file: shaders_gles/shader not found" << endl;
		ofExit(0);
	}
#else
	if (!shader.load("shaders/shader")) {
		cout << "Error: Can't load shader file: shaders/shader not found" << endl;
		ofExit(0);
	}
#endif

	// Create Octree
	octree.create(terrain.getMesh(0), 20);

	setupLander();

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
	emitter->sys->addForce(gravityForce);
	emitter->radius = 0.2f;
	emitter->rate = 45.0f;
	emitter->particleRadius = 3.0f;
	emitter->lifespan = 0.5f;
	emitter->groupSize = 50;
	emitter->particleVelocity = ofVec3f(0, -0.8f, 0);

	// Set up explosion particle system
	explosionForce = new ImpulseRadialForce(explosionMagnitude);
	explosionForce->applyOnce = true;

	explosionParticleSys = new ParticleSystem();
	explosionEmitter = new ParticleEmitter(explosionParticleSys);

	explosionEmitter->sys->addForce(explosionForce);

	explosionEmitter->type = RadialEmitter;
	explosionEmitter->particleRadius = 9.0f;
	explosionEmitter->lifespan = 4.0f;
	explosionEmitter->groupSize = 1300;
	explosionEmitter->particleVelocity = ofVec3f(0, 0, 0);
	explosionEmitter->oneShot = true;

	// Set up lighting
	ambientLight.setup();
	ambientLight.enable();
	ambientLight.setAreaLight(0.05, 0.05);
	ambientLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	ambientLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	ambientLight.setSpecularColor(ofFloatColor(1, 1, 1));

	ambientLight.setPosition(ofVec3f(0, 100, 0));

	float lightDistance = 10;

	landingArea1Light.setup();
	landingArea1Light.enable();
	landingArea1Light.setSpotlight();
	landingArea1Light.setScale(.05);
	landingArea1Light.setSpotlightCutOff(75);
	landingArea1Light.setAttenuation(0.25, .01, .01);
	landingArea1Light.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landingArea1Light.setDiffuseColor(ofColor::lightBlue);
	landingArea1Light.setSpecularColor(ofFloatColor(1, 1, 1));
	landingArea1Light.rotate(-90, ofVec3f(1, 0, 0));
	landingArea1Light.setPosition(landingArea1 + ofVec3f(0, lightDistance, 0));
	if (gameEnv == DESERT) {
		landingArea1Light.setPosition(landingArea1 + ofVec3f(0, 5.0, 0));
	}

	landingArea2Light.setup();
	landingArea2Light.enable();
	landingArea2Light.setSpotlight();
	landingArea2Light.setScale(.05);
	landingArea2Light.setSpotlightCutOff(75);
	landingArea2Light.setAttenuation(0.25, .01, .01);
	landingArea2Light.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landingArea2Light.setDiffuseColor(ofColor::lightBlue);
	landingArea2Light.setSpecularColor(ofFloatColor(1, 1, 1));
	landingArea2Light.rotate(-90, ofVec3f(1, 0, 0));
	landingArea2Light.setPosition(landingArea2 + ofVec3f(0, lightDistance, 0));
	if (gameEnv == DESERT) {
		landingArea2Light.setPosition(landingArea2 + ofVec3f(0, 4.0, 0));
	}

	landingArea3Light.setup();
	landingArea3Light.enable();
	landingArea3Light.setSpotlight();
	landingArea3Light.setScale(.05);
	landingArea3Light.setSpotlightCutOff(75);
	landingArea3Light.setAttenuation(0.25, .01, .01);
	landingArea3Light.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landingArea3Light.setDiffuseColor(ofColor::lightBlue);
	landingArea3Light.setSpecularColor(ofFloatColor(1, 1, 1));
	landingArea3Light.rotate(-90, ofVec3f(1, 0, 0));
	landingArea3Light.setPosition(landingArea3 + ofVec3f(0, lightDistance, 0));
	if (gameEnv == DESERT) {
		landingArea3Light.setPosition(landingArea3 + ofVec3f(0, 0.3, 0));
	}

	landerLight.setup();
	landerLight.enable();
	landerLight.setSpotlight(45, 10);
	landerLight.setAttenuation(1, .2, .02);
	landerLight.setDiffuseColor(ofColor::orange);
	landerLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landerLight.setSpecularColor(ofFloatColor(1, 1, 1));
	landerLight.rotate(-90, ofVec3f(1, 0, 0));
	landerLight.setPosition(lander->getPosition());

	// Set up cameras
	freeCam.setTarget(lander->getPosition());
	freeCam.setDistance(10);
	freeCam.setNearClip(.1);
	freeCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	freeCam.disableMouseInput();

	topCam.setNearClip(.1);
	topCam.setFov(65.5);
	topCam.setPosition(0, 15, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));

	trackingCam.setPosition(ofVec3f(20, 24, -15));
	trackingCam.setTarget(lander->getPosition());
	trackingCam.setDistance(10);
	trackingCam.setNearClip(0.1);
	trackingCam.setFov(65.5);
	trackingCam.disableMouseInput();

	onboardCam.setPosition(lander->getPosition());
	onboardCam.setTarget(lander->getPosition() + lander->getForwardUV());
	onboardCam.setDistance(10);
	onboardCam.setNearClip(0.1);
	onboardCam.setFov(65.5);
	onboardCam.disableMouseInput();

	theCam = &freeCam;
}

//--------------------------------------------------------------
void ofApp::update(){
	if (score == 30) {
		gamestate = ENDGAME;
	}

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
	if (gamestate == INGAME) {
		thrustForce->update(lander);
		tanForce->update(lander);

		if (fuel <= 0) {
			gamestate = ENDGAME;
		}
		else {
			// Reduce fuel if thrusters are being used
			float thrustMagnitude = thrustForce->getThrust().length();
			float tanMagnitude = tanForce->getTorque().length();
			if (thrustMagnitude != 0 || tanMagnitude != 0) {
				float dt = 1.0f / ofGetFrameRate();
				fuel -= dt;
			}
		}
	}
	else {
		emitter->stop();
	}
	if (gamestate != PREGAME) {
		turbForce->update(lander);
		gravityForce->update(lander);
		lander->integrate();
	}

	emitter->position = lander->getPosition();
	emitter->update();

	landerLight.setPosition(lander->getPosition());

	// Update cameras
	trackingCam.lookAt(lander->getPosition());
	onboardCam.setPosition(lander->getPosition());
	onboardCam.setTarget(lander->getPosition() + lander->getForwardUV());

	explosionEmitter->update();

	// Compute lander bounds
	ofVec3f min = lander->model.getSceneMin() + lander->getPosition();
	ofVec3f max = lander->model.getSceneMax() + lander->getPosition();
	Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	// Update collision boxes
	colBoxList.clear();
	octree.intersect(bounds, octree.root, colBoxList);

	// Handle lander collision with the terrain
	if (gamestate != PREGAME && colBoxList.size() >= 10) {
		// Apply impulse to the lander upon collision
		ofVec3f yNormal = ofVec3f(0, 1, 0);
		lander->velocity = (yNormal.dot(-lander->velocity) * yNormal) * 1.25;

		// Check if lander is on the lander area
		if (gamestate == INGAME) {
			// Explode if lander is too fast
			if (lander->velocity.length() >= 2.5f) {
				explosionForce->applied = false;
				explosionEmitter->position = lander->getPosition();
				explosionEmitter->start();
				shipExploded = true;
				gamestate = ENDGAME;
			}

			// Check if lander successfully landed
			if (lander->velocity.length() < 1.5f) {
				Vector3 la1 = Vector3(landingArea1.x, landingArea1.y, landingArea1.z);
				Vector3 la2 = Vector3(landingArea2.x, landingArea2.y, landingArea2.z);
				Vector3 la3 = Vector3(landingArea3.x, landingArea3.y, landingArea3.z);

				if (!area1Landed && bounds.inside(la1)) {
					area1Landed = true;
					score += 10;
					landingArea1Light.setDiffuseColor(ofColor::green);
				}
				else if (!area2Landed && bounds.inside(la2)) {
					area2Landed = true;
					score += 10;
					landingArea2Light.setDiffuseColor(ofColor::green);
				}
				else if (!area3Landed && bounds.inside(la3)) {
					area3Landed = true;
					score += 10;
					landingArea3Light.setDiffuseColor(ofColor::green);
				}
			}
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

	loadVbo();

	// Enable lighting
	ofEnableLighting();

	theCam->begin();

	ofPushMatrix();

	// Draw LEM and the terrain
	terrain.drawFaces();
	lander->model.drawFaces();

	if (bDisplayPoints) {                
		// display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
	}

	// Disable lighting
	ofDisableLighting();

	// draw shaded particles
	glDepthMask(GL_FALSE);

	ofSetColor(255, 100, 90);

	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	shader.begin();

	particleTexture.bind();
	vbo.draw(GL_POINTS, 0, (int)(emitter->sys->particles.size() + explosionEmitter->sys->particles.size()));
	particleTexture.unbind();

	shader.end();

	ofDisablePointSprites();
	ofDisableBlendMode();
	//ofEnableAlphaBlending();

	glDepthMask(GL_TRUE);

	// Draw lander and collision boxes
	ofNoFill();

	if (gamestate == PREGAME) {
		ofVec3f min = lander->model.getSceneMin() * 0.5 + lander->getPosition();
		ofVec3f max = lander->model.getSceneMax() * 0.5 + lander->getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		ofSetColor(ofColor::white);
		if (bLanderSelected) {
			ofSetColor(ofColor::red);
		}
		Octree::drawBox(bounds);

		ofSetColor(ofColor::lightBlue);
		for (int i = 0; i < colBoxList.size(); i++) {
			Octree::drawBox(colBoxList[i]);
		}
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
	ofDrawBitmapString("Score: " + std::to_string(score), ofGetWindowWidth() - 170, 45);

	if (gamestate == PREGAME) {
		string startText = "Press 'SPACEBAR' to start\n";
		const string longestLine = "You can drag the ship around before starting the game\n";
		startText += longestLine;
		textDisplay.drawString(
			startText,
			ofGetWindowWidth() / 2 - textDisplay.stringWidth(longestLine) / 2,
			ofGetWindowHeight() / 2
		);
	}
	else if (gamestate == ENDGAME) {
		string endText;
		if (shipExploded) {
			endText += "Your ship exploded!\n";
		}
		else if (fuel <= 0) {
			endText += "Your ship ran out of fuel!\n";
		}
		else {
			endText += "You win!\n";
		}
		endText += "Your score is " + ofToString(score) + ".\n";
		if (fuel >= 0) {
			endText += "Your remaining fuel is " + ofToString(fuel) + ".\n";
		}
		endText += "Press P to play again.";
		textDisplay.drawString(
			endText,
			ofGetWindowWidth() / 2 - textDisplay.stringWidth("MMMMMMMMMMMM") / 2,
			ofGetWindowHeight() / 2
		);
	}

	ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	keymap[key] = true;

	if (gamestate == PREGAME && key == ' ') {
		gamestate = INGAME;
	}

	if (gamestate != PREGAME && (key == 'p' || key == 'P')) {
		gamestate = PREGAME;
		// Reset lander and emitter positions, and variables
		free(lander);
		setupLander();
		fuel = 120;
		score = 0;
	}

	switch (key) {
	case 'C':
	case 'c':
		if (freeCam.getMouseInputEnabled()) freeCam.disableMouseInput();
		else freeCam.enableMouseInput();
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
	case 'r':
		freeCam.reset();
		freeCam.setTarget(lander->getPosition());
		freeCam.setDistance(10);
		freeCam.setNearClip(.1);
		freeCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
		freeCam.disableMouseInput();
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
	case OF_KEY_F1:
		theCam = &freeCam;
		break;
	case OF_KEY_F2:
		theCam = &trackingCam;
		freeCam.enableMouseInput();
		break;
	case OF_KEY_F3:
		theCam = &onboardCam;
		freeCam.enableMouseInput();
		break;
	case OF_KEY_ALT:
		freeCam.enableMouseInput();
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
		freeCam.disableMouseInput();
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
	// If moving camera, do not allow mouse interactions
	if (freeCam.getMouseInputEnabled()) {
		return;
	}

	if (bInDrag) {
		glm::vec3 landerPos = lander->getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, theCam->getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;

		landerPos += delta;
		lander->setPosition(ofVec3f(landerPos.x, landerPos.y, landerPos.z));
		mouseLastPos = mousePos;

		ofVec3f min = lander->model.getSceneMin() * 0.5 + lander->getPosition();
		ofVec3f max = lander->model.getSceneMax() * 0.5 + lander->getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	// If moving camera, do not allow mouse interactions
	if (freeCam.getMouseInputEnabled() || theCam != &freeCam) {
		return;
	}

	// Allow lander selection only in PREGAME state
	if (gamestate == PREGAME) {
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander->model.getSceneMin() * 0.5 + lander->getPosition();
		ofVec3f max = lander->model.getSceneMax() * 0.5 + lander->getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		bLanderSelected = bounds.intersect(
			Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 1 << 20
		);

		if (bLanderSelected) {
			mouseDownPos = getMousePointOnPlane(lander->getPosition(), theCam->getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bInDrag = false;
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
	free(gravityForce);
	free(emitter);
	free(particleSys);
	free(explosionEmitter);
	free(explosionParticleSys);
	free(lander);
}