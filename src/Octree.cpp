
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Simple Octree Implementation 11/10/2020
// 
//  Copyright (c) by Kevin M. Smith
//  Copying or use without permission is prohibited by law. 
//

#include "Octree.h"

//draw a box from a "Box" class  
//
void Octree::drawBox(const Box& box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh& mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
	//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh& mesh, const vector<int>& points,
	Box& box, vector<int>& pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh& mesh, const vector<int>& faces,
	Box& box, vector<int>& facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p, 3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
void Octree::subDivideBox8(const Box& box, vector<Box>& boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh& geo, int numLevels) {
	// initialize octree structure
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}

	// recursively buid octree
	level++;
	subdivide(mesh, root, numLevels, level);
}

void Octree::subdivide(const ofMesh& mesh, TreeNode& node, int numLevels, int level) {
	if (level >= numLevels) return;

	// subdvide algorithm implemented here

	// Subdivide box in node into 8 equal side boxes
	vector<Box> subboxes;
	subDivideBox8(node.box, subboxes);

	// Sort point data into each box
	for (int i = 0; i < subboxes.size(); i++) {
		vector<int> points;
		int pointCount = getMeshPointsInBox(mesh, node.points, subboxes[i], points);

		// if a child box contains at least 1 point, then add it to the tree
		if (pointCount > 0) {
			TreeNode newNode = TreeNode();
			newNode.box = subboxes[i];
			newNode.points = points;

			node.children.push_back(newNode);
		}
	}

	// Call subdivide again on all the non-leaf children nodes.
	for (int i = 0; i < node.children.size(); i++) {
		// If child has more than 1 point, it is not a leaf node.
		if (node.children[i].points.size() > 1) {
			subdivide(mesh, node.children[i], numLevels, level + 1);
		}
	}
}

/* intersect() function uses a ray and an octree, and selects the leaf node in the octree
 * that intersects with the ray */
bool Octree::intersect(const Ray& ray, const TreeNode& node, TreeNode& nodeRtn) {
	bool intersects = false;

	if (node.box.intersect(ray, 0, FLT_MAX)) {
		// If node has children, it is not a leaf node, thus use recursion
		if (node.children.size() > 0) {
			for (int i = 0; i < node.children.size(); i++) {
				if (intersect(ray, node.children[i], nodeRtn)) {
					intersects = true;
					break;
				}
			}
		}
		else {
			// Exit when a leaf node is selected
			nodeRtn = node;
			intersects = true;
		}
	}

	return intersects;
}

/* intersect() function uses a ray and an octree, and selects the node at depth level of numLevels
 * in the octree that intersects with the ray */
bool Octree::intersect(const Ray& ray, const TreeNode& node, int numLevels, int level, TreeNode& nodeRtn) {
	bool intersects = false;

	if (node.box.intersect(ray, 0, FLT_MAX)) {
		// If node has children and its depth level is not numLevels, then use recursion
		if (node.children.size() > 0 && level < numLevels) {
			for (int i = 0; i < node.children.size(); i++) {
				if (intersect(ray, node.children[i], numLevels, level + 1, nodeRtn)) {
					intersects = true;
					break;
				}
			}
		}
		else {
			// Exit when a node is either a leaf or is at depth numLevels.
			nodeRtn = node;
			intersects = true;
		}
	}

	return intersects;
}

/* intersect() function takes a box and returns a list of all leaf node boxes that intersect
 * with the given box. */
bool Octree::intersect(const Box& box, TreeNode& node, vector<Box>& boxListRtn) {
	bool intersects = false;

	// If boxes overlap, then they intersect.
	if (node.box.overlap(box)) {
		// If node of the box has children, then recursively call intersect on the node's children
		if (node.children.size() > 0) {
			for (int i = 0; i < node.children.size(); i++) {
				if (intersect(box, node.children[i], boxListRtn)) {
					return true;
				}
			}
		}
		else {
			// If the node is a leaf node, append its box to the list of boxes to return.
			boxListRtn.push_back(node.box);
		}
	}

	return intersects;
}

/* draw() draws the bounding boxes of each node in the octree up to the nodes of depth equivalent to numLevels. */
void Octree::draw(TreeNode& node, int numLevels, int level) {
	if (level >= numLevels) return;

	// Set color of the bounding box
	ofSetColor(colors[level % numLevels]);
	drawBox(node.box);

	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level + 1);
	}
}

// Optional to implement
void Octree::drawLeafNodes(TreeNode& node) {}