// author: Vanessa Tang

#pragma once

#include "ofMain.h"
#include <glm/gtx/intersect.hpp>
#include "ofxGui.h"

//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }

	//texture stuff
	virtual ofColor getColor(glm::vec3 point) {
		return ofColor::pink;
	}
	virtual glm::vec2 getIJCoords(glm::vec3 point, int numTiles) {
		return glm::vec2(0, 0);
	}
	virtual glm::vec2 getIJCoordsSpec(glm::vec3 point, int numTiles) {
		return glm::vec2(0, 0);
	}

	// any data common to all scene objects goes here
	glm::vec3 position = glm::vec3(0, 0, 0);

	// texture stuff
	void setTexture(ofImage theTexture) {
		texture = theTexture;
		textured = true;
	}
	void setSpec(ofImage theSpec) {
		specularTexture = theSpec;
	}

	// UI parameters
	bool isSelectable = true;

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;
	bool textured = false;
	ofImage texture;
	ofImage specularTexture;
	string name = "SceneObject";
};

// adds float intensity to SceneObject
class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) {
		position = p;
		intensity = i;
		name = "light";
	}

	Light() {
		name = "light";
	}

	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}

	void draw() {
		ofSetColor(ofColor::darkRed);
		ofDrawSphere(position, .1);
	}

	glm::vec3 getPosition() {
		return position;
	}

	float getIntensity() {
		return intensity;
	}
	void setIntensity(float theI) {
		intensity = theI;
	}

	float intensity = 0;
	float radius = 0.1;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; name = "sphere"; }
	Sphere() {
		name = "sphere";
	}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	void draw() {
		ofDrawSphere(position, radius);
	}
	void setRadius(float theR) {
		radius = theR;
	}

	float radius = 1.0;
};

//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::green, float w =
		20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		isSelectable = false;
		if (normal == glm::vec3(0, 1, 0))
			plane.rotateDeg(-90, 1, 0, 0);
		else if (normal == glm::vec3(0, -1, 0))
			plane.rotateDeg(90, 1, 0, 0);
		else if (normal == glm::vec3(1, 0, 0))
			plane.rotateDeg(90, 0, 1, 0);
		else if (normal == glm::vec3(-1, 0, 0))
			plane.rotateDeg(-90, 0, 1, 0);
	}
	Plane() {
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);
		isSelectable = false;
	}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	float sdf(const glm::vec3& p);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }
	void draw() {
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);
		// plane.drawWireframe();
		plane.draw();
	}

	// get the IJ coords from the intersection point
	//
	glm::vec2 getIJCoords(glm::vec3 point, int numTiles) {
		float planeWidth = this->width;
		float planeHeight = this->height;
		float u;
		float v;

		// map u
		u = ofMap(point.x, position.x - (planeWidth / 2), position.x + (planeWidth / 2), 0.0, 1.0);

		// map v depending on the normal
		// ground
		if (this->normal == glm::vec3(0, 1, 0)) {
			v = ofMap(point.z, position.z - (planeHeight / 2), position.z + (planeHeight / 2), 0.0, 1.0);
		}

		// wall
		if (this->normal == glm::vec3(0, 0, 1)) {
			v = ofMap(point.y, position.y - (planeHeight / 2), position.y + (planeHeight / 2), 0.0, 1.0);
		}
		u = u * numTiles;
		v = v * numTiles;

		// get the IJ coord from u and v
		float textureWidth = texture.getWidth();
		float textureHeight = texture.getHeight();
		int i = fmod(u * textureWidth - 0.5, textureWidth);
		int j = fmod(v * textureHeight - 0.5, textureHeight);

		//cout << "texture U: " << u << " texture V: " << v << " texture X: " << x << " texture y: " << y << "\n";
		return glm::vec2(i, j);
	}

	glm::vec2 getIJCoordsSpec(glm::vec3 point, int numTiles) {
		float planeWidth = this->width;
		float planeHeight = this->height;
		float u;
		float v;

		u = ofMap(point.x, position.x - (planeWidth / 2), position.x + (planeWidth / 2), 0.0, 1.0);

		// ground
		if (this->normal == glm::vec3(0, 1, 0)) {
			v = ofMap(point.z, position.z - (planeHeight / 2), position.z + (planeHeight / 2), 0.0, 1.0);
		}

		// wall
		if (this->normal == glm::vec3(0, 0, 1)) {
			v = ofMap(point.y, position.y - (planeHeight / 2), position.y + (planeHeight / 2), 0.0, 1.0);
		}
		u = u * numTiles;
		v = v * numTiles;

		float specWidth = specularTexture.getWidth();
		float specHeight = specularTexture.getHeight();
		int i = fmod(u * specWidth - 0.5, specWidth);
		int j = fmod(v * specHeight - 0.5, specHeight);

		//cout << "texture U: " << u << " texture V: " << v << " texture X: " << x << " texture y: " << y << "\n";
		return glm::vec2(i, j);
	}

	ofPlanePrimitive plane;
	glm::vec3 normal;
	float width = 20;
	float height = 20;
};

// view plane for render camera
// 
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}


	float width() {
		return (max.x - min.x);
	}
	float height() {
		return (max.y - min.y);
	}

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	glm::vec2 min, max;
};

//  render camera  - z axis aligned
//
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 10);
		aim = glm::vec3(0, 0, -1);
	}
	Ray getRay(float u, float v);
	void draw() { ofDrawBox(position, 1.0); };
	void drawFrustum();

	glm::vec3 aim;
	ViewPlane view;          // The camera viewplane, this is the view that we will render 
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
		void rayTrace();
		void drawGrid();
		bool mouseToDragPlane(int x, int y, glm::vec3& point);
		bool objSelected() { return (selected.size() ? true : false); };

		// Creating and Deleting Objects
		//
		void newSphere();
		void newLight();
		void deleteObj();

		// Ray tracing
		//
		ofColor lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse);
		ofColor phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power);
		bool inShadow(const Ray& r);

		// Anti Aliasing
		//
		void reSSAntiAlias();
		ofxIntSlider superSampleAmt;
		bool aaPrev = false;
		int aaRenderNum = 2; // keeps track of the number of times the filter has been reapplied
		void rayTraceMSAA();
		

		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera previewCam;
		ofCamera* theCam;    // set to current camera either mainCam or sideCam
		
		RenderCam renderCam;

		// output images
		//
		ofImage image;
		ofImage AAImage;
		ofImage reAAImage;
		ofImage MSAAImage;

		// scene components
		//
		vector<SceneObject*> scene;
		vector<SceneObject*> selected;
		vector<Light*> sceneLights;


		// gui components
		//
		ofxPanel gui;
		//sphere: size, color
		ofxFloatSlider sphereRadius;
		ofxColorSlider objColor;
		//lights: intensity
		ofxFloatSlider lightIntensity;
		//Texture: number of tiles
		ofxIntSlider numTilesSlider;
		

		// state
		bool bDrag = false;
		bool bHide = false;
		bool camMove = false;
		glm::vec3 lastPoint;

		// mouse position
		glm::vec3 mousePosition = glm::vec3(0, 1, 0);

		// output image
		int imageWidth = 2400;
		int imageHeight = 1600;
		int AAImageWidth;
		int AAImageHeight;
		int reAAImageWidth;
		int reAAImageHeight;
		int MSAAImageWidth = 1200;
		int MSAAImageHeight = 800;
};
