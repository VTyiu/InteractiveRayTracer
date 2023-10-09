// author: Vanessa Tang

#include "ofApp.h"
#include <glm/gtx/intersect.hpp>
#include "ofxGui.h"
#include <typeinfo>
#include <string>

// Intersect Ray with Plane  (wrapper on glm::intersect*)
//
bool Plane::intersect(const Ray& ray, glm::vec3& point, glm::vec3&
	normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal,
		dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width
			/ 2);
		glm::vec2 yrange = glm::vec2(position.y - width / 2, position.y + width
			/ 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z +
			height / 2);
		// horizontal 
		//
		if (normal == glm::vec3(0, 1, 0) || normal == glm::vec3(0, -1, 0)) {
			if (point.x < xrange[1] && point.x > xrange[0] && point.z <
				zrange[1] && point.z > zrange[0]) {
				insidePlane = true;
			}
		}
		// front or back
		//
		else if (normal == glm::vec3(0, 0, 1) || normal == glm::vec3(0, 0, -1))
		{
			if (point.x < xrange[1] && point.x > xrange[0] && point.y <
				yrange[1] && point.y > yrange[0]) {
				insidePlane = true;
			}
		}
		// left or right
		//
		else if (normal == glm::vec3(1, 0, 0) || normal == glm::vec3(-1, 0, 0))
		{
			if (point.y < yrange[1] && point.y > yrange[0] && point.z <
				zrange[1] && point.z > zrange[0]) {
				insidePlane = true;
			}
		}
	}
	return insidePlane;
}

// Convert (u, v) to (x, y, z) 
// We assume u,v is in [0, 1]
//
glm::vec3 ViewPlane::toWorld(float u, float v) {
	float w = width();
	float h = height();
	return (glm::vec3((u * w) + min.x, (v * h) + min.y, position.z));
}

// Get a ray from the current camera position to the (u, v) position on
// the ViewPlane
//
Ray RenderCam::getRay(float u, float v) {
	glm::vec3 pointOnPlane = view.toWorld(u, v);
	return(Ray(position, glm::normalize(pointOnPlane - position)));
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetBackgroundColor(ofColor::black);

	// gui
	gui.setup();
	gui.add(sphereRadius.setup("Sphere Radius", 2.0f, 0.1f, 5.0f));
	gui.add(objColor.setup("Sphere Color", ofColor(100, 100, 140), ofColor(0, 0), ofColor(255, 255)));
	gui.add(lightIntensity.setup("Light Intensity", 100.0f, 0.1f, 1000.0f));
	gui.add(superSampleAmt.setup("Anti-Alias Sample Size", 2, 1, 8));
	gui.add(numTilesSlider.setup("Number of Tiles", 3, 1, 10));

	// main cam
	mainCam.setDistance(13.0);
	mainCam.lookAt(glm::vec3(0, 3, 0));
	mainCam.setNearClip(.1);

	//camera that looks through render cam
	//previewCam
	previewCam.lookAt(glm::vec3(0, 0, 0));
	previewCam.setNearClip(.1);
	previewCam.setPosition(renderCam.position);

	theCam = &mainCam;
	
	ofSetDepthTest(true);


	// create an ofImage object for your target rendering image
	image.allocate(imageWidth, imageHeight, ofImageType::OF_IMAGE_COLOR);

	// texture application
	ofImage texture;

	/*Plane* groundPlane = new Plane(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), ofColor::aqua);
	scene.push_back(groundPlane);
	Plane* wallPlane = new Plane(glm::vec3(0, 0, -8), glm::vec3(0, 0, 1), ofColor::white);
	scene.push_back(wallPlane);*/

	Plane* groundPlane = new Plane(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), ofColor::aqua);
	texture.load("cobble.jpg");
	groundPlane->setTexture(texture); // 1250x1250
	texture.load("cobblespec.jpg");
	groundPlane->setSpec(texture);
	scene.push_back(groundPlane);

	Plane* wallPlane = new Plane(glm::vec3(0, 0, -8), glm::vec3(0, 0, 1), ofColor::white);
	texture.load("green.jpg");
	wallPlane->setTexture(texture);
	texture.load("greenspec.jpg");
	wallPlane->setSpec(texture);
	scene.push_back(wallPlane);

	// the spheres
	scene.push_back(new Sphere(glm::vec3(-3, 1, -5), 2.0f, ofColor::darkCyan));
	scene.push_back(new Sphere(glm::vec3(4, 1, 0), 1.0f, ofColor::darkGreen));
	scene.push_back(new Sphere(glm::vec3(0, 1, 0), 2.0f, ofColor::darkKhaki));
	//scene.push_back(new Plane(glm::vec3(0, -1, 0), glm::vec3(0, 1, 0), ofColor::aqua));
	//scene.push_back(new Plane(glm::vec3(0, 0, -8), glm::vec3(0, 0, 1), ofColor::white));

	sceneLights.push_back(new Light(glm::vec3(-5, 3, 3), 40.0f));
	sceneLights.push_back(new Light(glm::vec3(0, 5, -1), 50.0f));
	sceneLights.push_back(new Light(glm::vec3(3, 2, 5), 50.0f));


}

//--------------------------------------------------------------
void ofApp::update(){

}

// create a new sphere in the scene at the position of the mouse pointer
//
void ofApp::newSphere() {
	Sphere* addSphere = new Sphere(mousePosition, .5);
	scene.push_back(addSphere);
}

// create a new light in the scene at the position of the mouse pointer
//
void ofApp::newLight() {
	Light* addLight = new Light(mousePosition, 0.1f);
	//scene.push_back(addLight);
	sceneLights.push_back(addLight);
}

// delete selected object in the scene
//
void ofApp::deleteObj() {
	int i = 0;
	bool found = false;
	if (objSelected()) {
		// if obj selected is a sphere
		Light* selectedObj = dynamic_cast<Light*> (selected[0]);
		if (selectedObj == nullptr) {
			while (i < scene.size() && !found) {
				if (selected[0] == scene[i]) {
					found = true;
				}
				else
					i++;
			}
			scene.erase(scene.begin() + i);
		}

		// if obj selected is a light
		Light* selectedLight = dynamic_cast<Light*> (selected[0]);
		if (selectedLight != nullptr) {
			int j = 0;
			found = false;
			while (j < sceneLights.size() && !found) {
				if (selected[0] == sceneLights[j]) {
					found = true;
				}
				else
					j++;
			}
			sceneLights.erase(sceneLights.begin() + j);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	

	ofSetDepthTest(true);

	theCam->begin();

	for (int i = 0; i < scene.size(); i++) {
		ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}

	for (int i = 0; i < sceneLights.size(); i++) {
		ofSetColor(sceneLights[i]->diffuseColor);
		sceneLights[i]->draw();
	}

	theCam->end();
	ofSetDepthTest(false);
	// draw the GUI
	gui.draw();
}

// ray trace with multi sample anti aliasing
//
void ofApp::rayTraceMSAA() {
	MSAAImage.allocate(MSAAImageWidth, MSAAImageHeight, ofImageType::OF_IMAGE_COLOR);
	// go through all pixels of the output image
	for (int j = 0; j < MSAAImageHeight; j++) {
		for (int i = 0; i < MSAAImageWidth; i++) {

			// the distance between each ray vector
			float samplingSplit = 1.0 / superSampleAmt;

			// variables to average out the color per pixel
			glm::vec3 colorSum = glm::vec3(0, 0, 0);
			glm::vec3 theColorVec = glm::vec3(0, 0, 0);
			glm::vec3 resultColorVec = glm::vec3(0, 0, 0);
			ofColor resultColor;

			// divide each pixel into samples
			for (float xOffset = samplingSplit / 2; xOffset < 1.0; xOffset += samplingSplit) {
				for (float yOffset = samplingSplit / 2; yOffset < 1.0; yOffset += samplingSplit) {
					float u = (float(i) + xOffset) / float(MSAAImageWidth);
					float v = (float(j) + yOffset) / float(MSAAImageHeight);

					// ray trace
					Ray theRay = renderCam.getRay(u, v);
					float shortestDistance = std::numeric_limits<float>::infinity();
					float currentDistance;
					SceneObject* closestObj = NULL; // pointer
					bool hitSurface = false;
					ofColor objColor;
					glm::vec3 intersectPoint, intersectNormal, closeIntersect, closeNormal;

					for (int m = 0; m < scene.size(); m++) {
						//cout << scene[m]->diffuseColor << " ";
						//objColor = scene[m]->diffuseColor;

						if (scene[m]->intersect(theRay, intersectPoint, intersectNormal)) {
							hitSurface = true;
							currentDistance = glm::distance(renderCam.position, intersectPoint);
							if (currentDistance < shortestDistance) { // only use the color of the closest intersection
								shortestDistance = currentDistance;
								closeIntersect = intersectPoint;
								closeNormal = intersectNormal;
								closestObj = scene[m];
							}
						}
					}

					// if the ray does not hit an object
					if (hitSurface == false) {
						ofColor theColor = ofGetBackgroundColor();
						theColorVec = glm::vec3(theColor.r, theColor.g, theColor.b);
						colorSum = colorSum + theColorVec;
					}
					else { // the ray hits an object
						if (closestObj->textured == true) { // if the object is textured
							ofColor theColor;
							glm::vec2 textureCoords;
							glm::vec2 specCoords;
							textureCoords = closestObj->getIJCoords(closeIntersect, numTilesSlider);
							specCoords = closestObj->getIJCoordsSpec(closeIntersect, numTilesSlider);
							ofColor textureColor;
							ofColor specColor;

							textureColor = closestObj->texture.getColor(textureCoords.x, textureCoords.y);
							specColor = closestObj->specularTexture.getColor(specCoords.x, specCoords.y);
							theColor = phong(closeIntersect, closeNormal, textureColor, specColor, 1000.0);
							theColorVec = glm::vec3(theColor.r, theColor.g, theColor.b);
							// add the color to colorSum
							colorSum = colorSum + theColorVec;
						}
						else { // if the object is not textured

							ofColor theColor;
							theColor = phong(closeIntersect, closeNormal, closestObj->diffuseColor, closestObj->specularColor, 1000.0);
							theColorVec = glm::vec3(theColor.r, theColor.g, theColor.b);
							// add the color to colorSum
							colorSum = colorSum + theColorVec;
						}
					}
				}
			}
			// average out the colors
			resultColorVec = colorSum / (superSampleAmt * superSampleAmt);
			resultColor = ofColor(resultColorVec[0], resultColorVec[1], resultColorVec[2]);
			MSAAImage.setColor(i, MSAAImageHeight - j - 1, resultColor);
		}
	}

	MSAAImage.save("MSAA Render.jpg");
	cout << "Multi-sample image done rendering" << endl;
}

// ray trace with SSAA
//
void ofApp::rayTrace() {

	// go through all pixels of output image
	for (int j = 0; j < imageHeight; j++) { // row
		for (int i = 0; i < imageWidth; i++) { // col

			// one ray per pixel
			float u = (float(i) + 0.5) / float(imageWidth); // pixel to image mapping // horizontal
			float v = (float(j) + 0.5) / float(imageHeight); // vertical

			Ray theRay = renderCam.getRay(u, v);
			float shortestDistance = std::numeric_limits<float>::infinity();
			float currentDistance;
			SceneObject* closestObj = NULL; // pointer
			bool hitSurface = false;
			ofColor objColor;
			glm::vec3 intersectPoint, intersectNormal, closeIntersect, closeNormal;

			for (int m = 0; m < scene.size(); m++) {
				//cout << scene[m]->diffuseColor << " ";
				//objColor = scene[m]->diffuseColor;

				if (scene[m]->intersect(theRay, intersectPoint, intersectNormal)) {
					hitSurface = true;
					currentDistance = glm::distance(renderCam.position, intersectPoint);
					if (currentDistance < shortestDistance) { // only use the color of the closest intersection
						shortestDistance = currentDistance;
						closeIntersect = intersectPoint;
						closeNormal = intersectNormal;
						closestObj = scene[m];
					}
				}
			}
			if (hitSurface == false) { // if the ray does not intersect with an object
				image.setColor(i, imageHeight - j - 1, ofGetBackgroundColor());
			}
			else { // if the ray intersects with an object
				ofColor color;
				if (closestObj->textured == true) { // if the object is textured
					glm::vec2 textureCoords;
					glm::vec2 specCoords;
					textureCoords = closestObj->getIJCoords(closeIntersect, numTilesSlider);
					specCoords = closestObj->getIJCoordsSpec(closeIntersect, numTilesSlider);
					ofColor textureColor;
					ofColor specColor;
					textureColor = closestObj->texture.getColor(textureCoords.x, textureCoords.y);
					specColor = closestObj->specularTexture.getColor(specCoords.x, specCoords.y);
					color = phong(closeIntersect, closeNormal, textureColor, specColor, 1000.0);
					image.setColor(i, imageHeight - j - 1, color);
				}
				else { // if the obj is not textured
					color = closestObj->diffuseColor;
					color = phong(closeIntersect, closeNormal, closestObj->diffuseColor, closestObj->specularColor, 1000.0);
					image.setColor(i, imageHeight - j - 1, color);

				}
			}
		}
	}

	image.save("Full Render.jpg");
	cout << "Original image done rendering" << endl;

	// check if SS anti aliasing is allowed with the chosen sample size
	bool antiAliasAllowed = false;
	float superSampleFl = (float)superSampleAmt;
	if ((fmod(imageWidth, superSampleFl)) == 0 && (fmod(imageHeight, superSampleFl) == 0))
		antiAliasAllowed = true;

	if (antiAliasAllowed == true) {
		cout << "Anti-alias render allowed" << endl;
	}
	else {
		cout << "Anti-alias render not allowed.  The original image is " << imageWidth << "x" << imageHeight << ".  Pick a super sample size that divides the size evenly." << endl;
		cout << endl;
		return;
	}

	// set image height and width of image with AA
	// with SSAA, the new image height and width is smaller than the original
	AAImageHeight = imageHeight / superSampleAmt;
	AAImageWidth = imageWidth / superSampleAmt;
	AAImage.allocate(AAImageWidth, AAImageHeight, ofImageType::OF_IMAGE_COLOR);

	// Super Sample Anti Aliasing
	int sampleNum = superSampleAmt;
	for (int j = 0; j < AAImageHeight; j++) { // row
		for (int i = 0; i < AAImageWidth; i++) { // col
			glm::vec3 colorSum = glm::vec3(0, 0, 0);
			glm::vec3 theColorVec = glm::vec3(0, 0, 0);
			glm::vec3 resultColorVec = glm::vec3(0, 0, 0);
			ofColor resultColor;

			// for every sample size x sample size pixels in the original render, the colors are averaged out into one pixel in the new SSAA image.
			for (int v = 0; v < sampleNum; v++) {
				for (int u = 0; u < sampleNum; u++) {
					ofColor theColor = image.getColor(i * sampleNum + fmod(u, sampleNum), j * sampleNum + fmod(v, sampleNum));
					theColorVec = glm::vec3(theColor.r, theColor.g, theColor.b);

					// add up colors
					colorSum = colorSum + theColorVec;
				}
			}

			resultColorVec = colorSum / (sampleNum * sampleNum);
			resultColor = ofColor(resultColorVec[0], resultColorVec[1], resultColorVec[2]);
			AAImage.setColor(i, j, resultColor);
		}
	}

	AAImage.save("SSAA Render x1.jpg");
	aaRenderNum = 2; // reset the number of times SSAA has been applied to the current render
	cout << "Supersample anti-aliasing image done rendering" << endl;

	// make full size image from super sample image
	// not really needed, just here to showcase how the image size decreases with each render of the SSAA filter which is why it is so expensive
	for (int j = 0; j < imageHeight; j++) {
		for (int i = 0; i < imageWidth; i++) {

			for (int b = 0; b < sampleNum; b++) {
				for (int a = 0; a < sampleNum; a++) {

					float u;
					float v;
					u = (int)ofMap(i, 0.0, imageWidth, 0.0, AAImageWidth);
					v = (int)ofMap(j, 0.0, imageHeight, 0.0, AAImageHeight);

					ofColor theColor = AAImage.getColor(u, v);
					image.setColor(i, j, theColor);
				}
			}
		}
	}
	image.save("SSAA Render Expanded.jpg");
	cout << "SSAA render expanded image done rendering" << endl;
	
	cout << endl;

	aaPrev = true;
}

// Reapply the SSAA filter to smooth out pixels even more
//
void ofApp::reSSAntiAlias() {
	// Do not allow the user to call this function if the scene has not been rendered before
	if (aaPrev == false) {
		cout << "Scene not rendered yet. Anti-alias render can only be refined after first render." << endl;
		return;
	}

	// Super Sample Anti Aliasing
	int sampleNum = superSampleAmt;
	reAAImageHeight = AAImageHeight / sampleNum;
	reAAImageWidth = AAImageWidth / sampleNum;
	reAAImage.allocate(reAAImageWidth, reAAImageHeight, ofImageType::OF_IMAGE_COLOR);
	for (int j = 0; j < reAAImageHeight; j++) { // row
		for (int i = 0; i < reAAImageWidth; i++) { // col
			glm::vec3 colorSum = glm::vec3(0, 0, 0);
			glm::vec3 theColorVec = glm::vec3(0, 0, 0);
			glm::vec3 resultColorVec = glm::vec3(0, 0, 0);
			ofColor resultColor;

			for (int v = 0; v < sampleNum; v++) {
				for (int u = 0; u < sampleNum; u++) {

					ofColor theColor = AAImage.getColor(i * sampleNum + fmod(u, sampleNum), j * sampleNum + fmod(v, sampleNum));
					theColorVec = glm::vec3(theColor.r, theColor.g, theColor.b);

					// add up colors
					colorSum = colorSum + theColorVec;
				}
			}

			resultColorVec = colorSum / (sampleNum * sampleNum);
			resultColor = ofColor(resultColorVec[0], resultColorVec[1], resultColorVec[2]);
			reAAImage.setColor(i, j, resultColor);
		}
	}
	string aaRenderNumString = to_string(aaRenderNum);
	string aaImageOutput = "SSAA Render x" + aaRenderNumString + ".jpg";
	reAAImage.save(aaImageOutput);
	cout << "Supersample anti-alias x" << aaRenderNumString << " image done rendering" << endl;
	aaRenderNum++;
	cout << "Remember to re-render if you change the scene" << endl;
	AAImage = reAAImage;
	AAImageWidth = reAAImageWidth;
	AAImageHeight = reAAImageHeight;
	cout << endl;
}

ofColor ofApp::lambert(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse) {
	// ofColor L = surface color * intensity * max(0, n * 1 for cos theta)
	// l is computed by subtracting the intersection point of the ray and surface from the light source position
	// v, l, and n all must be unit vectors

	// where p is the point of intersection, norm is the normal and diffuse is the diffuse color of the scene 
	// object.Use the math described in the lecture(and text) to implement the shading function(the shader).

	ofColor theLambert;
	ofColor ambient = 0.3f * diffuse * 2.0f;
	theLambert = ambient;

	for (auto light : sceneLights) {
		glm::vec3 n = glm::normalize(norm);
		glm::vec3 l = glm::normalize(light->position - p);
		glm::vec3 r = light->position - p;
		//float r = glm::length(l);
		theLambert += diffuse * light->intensity / pow(r.length(), 2) * glm::max(0.0f, glm::dot(n, l));
		//theLambert += ambient;
	}

	return theLambert;

}

ofColor ofApp::phong(const glm::vec3& p, const glm::vec3& norm, const ofColor diffuse, const ofColor specular, float power) {
	//ofColor color = ofColor::black;
	ofColor ambient = 0.3f * diffuse * 1.0f;
	ofColor color = ambient;
	// for each pixel, loop through all the lights, add up values from phong function for light value of pixel
	// if pixel is in a shadow, color is black

	for (auto light : sceneLights) {
		glm::vec3 n = glm::normalize(norm);
		glm::vec3 l = glm::normalize(light->position - p);
		glm::vec3 v = glm::normalize(renderCam.position - p);
		glm::vec3 h = glm::normalize((v + l) / glm::length(v + l));
		glm::vec3 r = light->position - p;
		ofColor theLambert, thePhong;
		glm::vec3 intersectPoint, intersectNormal;
		//float r = glm::length(l);

		if (!inShadow(Ray(p, l))) {
			// function from slides and textbook
			theLambert = diffuse * light->intensity / glm::pow(r.length(), 2) * glm::max(0.0f, glm::dot(n, l));
			thePhong = specular * light->intensity / glm::pow(r.length(), 2) * glm::max(0.0f, glm::pow(glm::dot(n, h), power));
			color += (theLambert + thePhong);
		}
	}
	return color;
}

bool ofApp::inShadow(const Ray& theRay) {
	float eps = .01; // offset
	for (auto obj : scene) {

		Plane* thePlane = dynamic_cast<Plane*> (obj);
		if (thePlane == nullptr) {
			glm::vec3 point, normal;
			if (obj->intersect(Ray(theRay.p + theRay.d * eps, theRay.d), point, normal))
				return true;
		}

	}
	return false;
}

// Pressing Keys
//

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'a':
		reSSAntiAlias();
		break;
	case 's':
		newSphere();
		break;
	case 'l':
		newLight();
		break;
	case 'd':
		deleteObj();
		break;
	case 'm':
		rayTraceMSAA();
		break;
	case 'r':
		rayTrace();
		break;
	case '1':
		theCam = &mainCam;
		break;
	case '2':
		theCam = &previewCam;
		break;
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case ' ':
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	mouseToDragPlane(x, y, mousePosition);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		selected[0]->position += (point - lastPoint);
		lastPoint = point;
	}
}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3& point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}

	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {

	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;
	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject*> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}

	// check for selection of scene objects
	//
	for (int i = 0; i < sceneLights.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (sceneLights[i]->isSelectable && sceneLights[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(sceneLights[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject* selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}

	if (selectedObj) {
		selected.push_back(selectedObj);

		// if the selected object is a sphere
		Sphere* selectedSphere = dynamic_cast<Sphere*> (selectedObj);
		if (selectedSphere != nullptr) {
			selectedSphere->radius = sphereRadius;
			selectedSphere->diffuseColor = objColor;
		}

		// if the selected object is a light
		Light* selectedLight = dynamic_cast<Light*> (selectedObj);
		if (selectedLight != nullptr) {
			selectedLight->intensity = lightIntensity;
		}

		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
		cout << "selected obj position: " << selectedObj->position << endl;
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bDrag = false;
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
