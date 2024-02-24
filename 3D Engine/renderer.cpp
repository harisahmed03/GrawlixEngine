#define NOMINMAX
#define _USE_MATH_DEFINES
#include "renderer.h"
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <iterator>

namespace haris {

	struct globalSettings {

	};
	using std::min;
	using std::max;

	float* pDepthBuffer = nullptr;

	mat4x4 matProj;

	std::list<mesh> myMeshes;
	std::list<mesh> frequencyMeshes;
	std::list<mesh> pointLightMeshes;

	Camera myCamera;

	float sensitivity = 25.0f;

	int screenWidth, screenHeight;

	bool clippingEnabled = false;

	DirectionalLight directionalLight;

	float timeElapsed;

	//Create init function
	Renderer::Renderer() {
		buffer = {};
		clearColor = { 255, 255, 255 };
		screenWidth = 800;
		screenHeight = 800;

		pDepthBuffer = new float[screenHeight * screenWidth];
		clearDepthBuffer(pDepthBuffer, screenWidth, screenHeight);

		matProj = Matrix_MakeProjection(90.0f, 1.0f, 0.1f, 1000.0f);

		setScene();
	}

	void haris::Renderer::setScene() {
		
		//Cube
		mesh meshCube;
		meshCube.loadFromObjectFile("Icosphere.txt");
		if (meshCube.tris.size() == 0) {
			myPrint("Could not load an object");
		}
		else {
			meshCube.randomizeTriColors();
			meshCube.coordinates = { 0, 5, 15, 0 };
			meshCube.rotation = { 0, 0, 0, 0 };
			meshCube.drawShaded = true;
			//meshCube.drawWireframe = true;
			meshCube.linkVolume = true;
			//meshCube.linkTheta = true;
			myMeshes.push_back(meshCube);
		}

		//Ground
		mesh meshFlatGround;
		meshFlatGround.loadFromObjectFile("FlatGround.txt");
		if (meshCube.tris.size() == 0) {
			exit(0);
		}
		meshFlatGround.randomizeTriColors();
		meshFlatGround.scale = { 1, 1, 1, 0 };
		//meshFlatGround.drawFilled = true;
		meshFlatGround.drawShaded = true;
		meshFlatGround.drawWireframe = false;
		//myMeshes.push_back(meshFlatGround);

		//Bar used for frequency display
		mesh meshBar;
		meshBar.loadFromObjectFile("Cube.txt");
		if (meshBar.tris.size() == 0) {
			myPrint("Could not load an object");
		}
		else {
			meshBar.randomizeTriColors();
			meshBar.coordinates = { 0, 0, 60, 0 };
			meshBar.scale = { 1, 1, 1, 0 };
			//meshBar.drawWireframe = true;
			meshBar.drawFilled = true;
			//meshBar.drawShaded = true;
			//meshBar.linkTheta = true;
			frequencyMeshes.push_back(meshBar);
		}

		mesh meshGun;
		meshGun.loadFromObjectFile("Gun.txt");
		if (meshGun.tris.size() == 0) {
			myPrint("Could not load an object");
		}
		else {
			//meshGun.randomizeTriColors();
			meshGun.coordinates = { 0, 0, 30, 0 };
			meshGun.scale = { 1, 1, 1, 0 };
			meshGun.drawShaded = true;
			//meshGun.drawWireframe = true;
			//myMeshes.push_back(meshGun);
		}

		mesh meshLightDirection;
		meshLightDirection.loadFromObjectFile("Arrow.txt");
		meshLightDirection.coordinates = { 30, 0, 0 };
		meshLightDirection.drawFilled = true;
		meshLightDirection.randomizeTriColors();
		meshLightDirection.rotation = { 0, -90, 0, 0 };
		meshLightDirection.moveMesh = true;
		//meshLightDirection.scale = { 0.1, 0.1, 0.1 };
		myMeshes.push_back(meshLightDirection);

	}

	void Renderer::RenderScene(float& delta, float& vol_l, float& vol_r, float* freq, int& numBars, float& hertz) {
		
		timeElapsed += delta;
		mat4x4 matView = GetCameraViewMatrix(delta);

		RotateDirectionalLight({ timeElapsed * 50, timeElapsed * 50, timeElapsed * 50 });

		for (auto& m : frequencyMeshes) {
			display3DFrequencyBars(m, matView, delta, vol_l, vol_r, freq, numBars);
		}
		
		for (auto& m : myMeshes) {
			if (m.moveMesh && myCamera.lockMovement) {
				MoveMesh(m, delta);
			}
			draw3dMesh(m, matView, delta, vol_l, vol_r);
		}
		
		if (haris::Input::isKeyPressed(H_O)) {
			for (auto& m : frequencyMeshes) {
				m.randomizeTriColors();
			}
		}

		if (haris::Input::isKeyPressed(H_I)) {
			for (auto& m : myMeshes) {
				m.randomizeTriColors();
			}
		}

		if (haris::Input::isKeyPressed(H_K)) {
			numBars = numBars - 1;
		}
		else if (haris::Input::isKeyPressed(H_L)) {
			numBars = numBars + 1;
		}

		//display2DFrequencyBars(freq);

		//Clear depth buffer
		clearDepthBuffer(pDepthBuffer, screenWidth, screenHeight);
	}

	void Renderer::MoveMesh(mesh& myMesh, float& deltaTime) {
		float x = 0;
		float y = 0;
		float z = 0;

		float moveSpeed = 30;

		if (haris::Input::isKeyPressed(H_A))
			x = 1;
		else if (haris::Input::isKeyPressed(H_D))
			x = -1;

		if (haris::Input::isKeyPressed(H_SPACE))
			y = 1;
		else if (haris::Input::isKeyPressed(H_SHIFT))
			y = -1;

		if (haris::Input::isKeyPressed(H_W))
			z = 1;
		else if (haris::Input::isKeyPressed(H_S))
			z = -1;

		if (x == 0 && y == 0 && z == 0) {
			return;
		}

		vec3d position = { x * moveSpeed * deltaTime, y * moveSpeed * deltaTime, z * moveSpeed * deltaTime };
		myMesh.coordinates = Vector_Add(myMesh.coordinates, position);
		myMesh.hasChanged = true;
	}

	mat4x4 Renderer::GetCameraViewMatrix(float deltaTime) {

		if (haris::Input::isKeyPressed(H_L))
			myCamera.lockMovement = !myCamera.lockMovement;

		float x = 0;
		float y = 0;
		float z = 0;
		float horizontalYaw = 0;
		float verticalYaw = 0;

		if (myCamera.lockMovement == false) {
			
			if (haris::Input::isKeyPressed(H_A))
				x = -1;
			else if (haris::Input::isKeyPressed(H_D))
				x = 1;

			if (haris::Input::isKeyPressed(H_SPACE))
				y = 1;
			else if (haris::Input::isKeyPressed(H_SHIFT))
				y = -1;

			if (haris::Input::isKeyPressed(H_W))
				z = 1;
			else if (haris::Input::isKeyPressed(H_S))
				z = -1;

			if (haris::Input::isKeyPressed(H_RIGHT))
				horizontalYaw = 1;
			else if (haris::Input::isKeyPressed(H_LEFT))
				horizontalYaw = -1;

			if (haris::Input::isKeyPressed(H_DOWN))
				verticalYaw = 1;
			else if (haris::Input::isKeyPressed(H_UP))
				verticalYaw = -1;

			if (haris::Input::isKeyPressed(H_Z))
				myCamera.mouseControls = !myCamera.mouseControls;
		}

		if (myCamera.mouseControls) {
			int mouseX = Input::getMousePosition().x - (screenWidth / 2);
			int mouseY = Input::getMousePosition().y - (screenHeight / 2);

			float outputX = mapOneRangeToAnother(mouseX, -(screenWidth / 2), (screenWidth / 2), -180, 180, 1);
			float outputY = mapOneRangeToAnother(mouseY, -(screenHeight / 2), (screenHeight / 2), -180, 180, 1);
			myCamera.horizontalYaw = outputX;
			myCamera.verticalYaw = outputY;
		}
		else {
			myCamera.horizontalYaw += horizontalYaw * myCamera.lookSpeed * deltaTime;
			myCamera.verticalYaw += verticalYaw * myCamera.lookSpeed * deltaTime;
		}

		if (myCamera.verticalYaw > 90) {
			myCamera.verticalYaw = 90;
		}
		else if (myCamera.verticalYaw < -90) {
			myCamera.verticalYaw = -90;
		}
		
		vec3d vUp = { 0,1,0 };
		vec3d vTarget = { 0, 0, 1 };

		mat4x4 matCameraRotX = Matrix_MakeRotationX(myCamera.verticalYaw);
		mat4x4 matCameraRotY = Matrix_MakeRotationY(myCamera.horizontalYaw);
		myCamera.vLookDir = Matrix_MultiplyVector(matCameraRotX, vTarget);
		myCamera.vLookDir = Matrix_MultiplyVector(matCameraRotY, myCamera.vLookDir);

		vec3d vForward = Vector_Normalise(myCamera.vLookDir);
		vec3d vRight = Vector_CrossProduct(vForward, vUp);
		vRight = Vector_Mul(vRight, myCamera.moveSpeed * deltaTime * x);
		vForward = Vector_Mul(vForward, myCamera.moveSpeed * deltaTime * z);
		vec3d vUp2 = Vector_Mul(vUp, myCamera.moveSpeed * deltaTime * y);

		myCamera.vCamera = Vector_Add(myCamera.vCamera, vRight);
		myCamera.vCamera = Vector_Add(myCamera.vCamera, vForward);
		myCamera.vCamera = Vector_Add(myCamera.vCamera, vUp2);

		vTarget = Vector_Add(myCamera.vCamera, myCamera.vLookDir);

		mat4x4 matCamera = Matrix_PointAt(myCamera.vCamera, vTarget, vUp);

		return Matrix_QuickInverse(matCamera);
	}

	void Renderer::RotateDirectionalLight(vec3d rotation) {
		mat4x4 RotX = Matrix_MakeRotationX(rotation.x);
		mat4x4 RotY = Matrix_MakeRotationY(rotation.y);
		mat4x4 RotZ = Matrix_MakeRotationY(rotation.z);

		vec3d vTarget = { 0, 0, 1 };
		directionalLight.directionalVector = Matrix_MultiplyVector(RotX, vTarget);
		directionalLight.directionalVector = Matrix_MultiplyVector(RotY, directionalLight.directionalVector);
		directionalLight.directionalVector = Matrix_MultiplyVector(RotZ, directionalLight.directionalVector);
	}

	void Renderer::transformTris(mesh& myMesh, float& theta, float& vol_l, float& vol_r) {

		if (!myMesh.hasChanged) {	//If the mesh hasn't changed AND it isn't linked to any audio vars
			if (!myMesh.linkFrequency && !myMesh.linkPitch && !myMesh.linkVolume && !myMesh.linkTheta && !myMesh.hasChanged) {
				return;
			}
		}

		mat4x4 matRotX;
		mat4x4 matRotY;
		mat4x4 matRotZ;

		if (myMesh.linkTheta) {
			matRotX = Matrix_MakeRotationX(myMesh.rotation.x + (theta));
			matRotY = Matrix_MakeRotationY(myMesh.rotation.y + (theta));
			matRotZ = Matrix_MakeRotationZ(myMesh.rotation.z + (theta));
		}
		else {
			matRotX = Matrix_MakeRotationX(myMesh.rotation.x);
			matRotY = Matrix_MakeRotationY(myMesh.rotation.y);
			matRotZ = Matrix_MakeRotationZ(myMesh.rotation.z);
		}

		mat4x4 matScale;

		if (myMesh.linkVolume) {
			float averageVolume = std::abs((vol_l + vol_r) / 2) * sensitivity;
			matScale = Matrix_MakeScale(myMesh.scale.x + averageVolume, myMesh.scale.y + averageVolume, myMesh.scale.z + averageVolume);
		}
		else {
			matScale = Matrix_MakeScale(myMesh.scale.x, myMesh.scale.y, myMesh.scale.z);
		}

		mat4x4 matTrans = Matrix_MakeTranslation(-myMesh.coordinates.x, myMesh.coordinates.y, myMesh.coordinates.z);

		mat4x4 matWorld = Matrix_MultiplyMatrix(matScale, matRotX);	//Scale and rotate around origin
		matWorld = Matrix_MultiplyMatrix(matWorld, matRotY);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);	//Translate away from origin

		std::vector<triangle> myTrisTransformed;

		//draw triangles
		for (auto tri : myMesh.tris) {
			triangle triProjected, triTransformed, triViewed;

			triTransformed.p[0].coordinates = Matrix_MultiplyVector(matWorld, tri.p[0].coordinates);
			triTransformed.p[1].coordinates = Matrix_MultiplyVector(matWorld, tri.p[1].coordinates);
			triTransformed.p[2].coordinates = Matrix_MultiplyVector(matWorld, tri.p[2].coordinates);
			triTransformed.fillColor = tri.fillColor;
			triTransformed.outlineColor = tri.outlineColor;

			myTrisTransformed.push_back(triTransformed);
		}

		myMesh.trisTransformed = myTrisTransformed;
		myMesh.hasChanged = false;
	}
	
	void Renderer::draw3dMesh(mesh& myMesh, mat4x4& matView, float& delta, float& vol_l, float& vol_r) {
		
		transformTris(myMesh, delta, vol_l, vol_r);

		std::vector<triangle> vecTrianglesToRaster;

		//draw triangles
		for (auto tri : myMesh.trisTransformed) {
			triangle triProjected, triViewed;

			vec3d normal, p0NormalisedNormal, line1, line2;
			line1 = Vector_Sub(tri.p[1].coordinates, tri.p[0].coordinates);
			line2 = Vector_Sub(tri.p[2].coordinates, tri.p[0].coordinates);
			normal = Vector_CrossProduct(line1, line2);
			p0NormalisedNormal = Vector_Normalise(normal);

			vec3d vCameraRay = Vector_Sub(tri.p[0].coordinates, myCamera.vCamera);

			if(Vector_DotProduct(p0NormalisedNormal, vCameraRay) < 0.0f)
			{
				triViewed.p[0].coordinates = Matrix_MultiplyVector(matView, tri.p[0].coordinates);
				triViewed.p[1].coordinates = Matrix_MultiplyVector(matView, tri.p[1].coordinates);
				triViewed.p[2].coordinates = Matrix_MultiplyVector(matView, tri.p[2].coordinates);

				int nClippedTriangles = 0;
				triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				for (int n = 0; n < nClippedTriangles; n++)
				{
					//Apply projection matrix
					triProjected.p[0].coordinates = Matrix_MultiplyVector(matProj, clipped[n].p[0].coordinates);
					triProjected.p[1].coordinates = Matrix_MultiplyVector(matProj, clipped[n].p[1].coordinates);
					triProjected.p[2].coordinates = Matrix_MultiplyVector(matProj, clipped[n].p[2].coordinates);

					//Normalize
					triProjected.p[0].coordinates = Vector_Div(triProjected.p[0].coordinates, triProjected.p[0].coordinates.w);
					triProjected.p[1].coordinates = Vector_Div(triProjected.p[1].coordinates, triProjected.p[1].coordinates.w);
					triProjected.p[2].coordinates = Vector_Div(triProjected.p[2].coordinates, triProjected.p[2].coordinates.w);

					vec3d vOffsetView = { 1,1,0 };

					triProjected.p[0].coordinates = Vector_Add(triProjected.p[0].coordinates, vOffsetView);
					triProjected.p[1].coordinates = Vector_Add(triProjected.p[1].coordinates, vOffsetView);
					triProjected.p[2].coordinates = Vector_Add(triProjected.p[2].coordinates, vOffsetView);

					float myScale = 0.5 * (screenWidth - 1);
					triProjected.p[0].coordinates.x *= myScale; triProjected.p[0].coordinates.y *= myScale; triProjected.p[0].coordinates.z *=  myScale;
					triProjected.p[1].coordinates.x *= myScale; triProjected.p[1].coordinates.y *= myScale; triProjected.p[1].coordinates.z *= myScale;
					triProjected.p[2].coordinates.x *= myScale; triProjected.p[2].coordinates.y *= myScale; triProjected.p[2].coordinates.z *= myScale;
					
					triProjected.fillColor = tri.fillColor;
					triProjected.outlineColor = tri.outlineColor;

					float facingRatio = std::fmax(directionalLight.ambientLightingIntensity, Vector_DotProduct(p0NormalisedNormal, directionalLight.directionalVector));
					triProjected.p[0].h = facingRatio;
					triProjected.p[1].h = facingRatio;
					triProjected.p[2].h = facingRatio;

					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}

		for (auto& triToRaster : vecTrianglesToRaster)
		{
			triangle clipped[2];
			std::list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)800 - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)800 - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}

			for (auto& t : listTriangles) {

				if (myMesh.drawWireframe) {
					drawTriangle({ (int)t.p[0].coordinates.x, (int)t.p[0].coordinates.y },
						{ (int)t.p[1].coordinates.x, (int)t.p[1].coordinates.y },
						{ (int)t.p[2].coordinates.x, (int)t.p[2].coordinates.y }, t.outlineColor);
				}
				if (myMesh.drawFilled) {
					drawFilledTriangle(t, t.fillColor);
				}
				else if (myMesh.drawShaded) {
					drawShadedTriangle(t, t.fillColor);
				}
			}
		}
	}

	void Renderer::display3DFrequencyBars(mesh& barMesh, mat4x4& matView, float& delta, float& vol_l, float& vol_r, float* freq, int& numBars) {

		mat4x4 matRotZ = Matrix_MakeRotationZ( barMesh.rotation.z);
		mat4x4 matRotX = Matrix_MakeRotationX( delta + barMesh.rotation.x);

		for (int b = 0; b < numBars; b++) {
			mesh myMesh = barMesh;

			myMesh.scale = { myMesh.scale.x, myMesh.scale.y + std::abs(freq[b]), myMesh.scale.z };	//spreads bars across X axis
			myMesh.coordinates = { (numBars / 2 - b) * (myMesh.scale.x * 2) + myMesh.coordinates.x, myMesh.coordinates.y, myMesh.coordinates.z };

			draw3dMesh(myMesh, matView, delta, vol_l, vol_r);
		}
	}
	
	void Renderer::display2DFrequencyBars(float* freq) {
		int numBars = 100;
		int barWidth = 5;
		int sens = 10;
		int startX = 100;

		for (int i = 0; i < numBars; i++) {
			Rect bar = { startX + (barWidth * i), 300, barWidth, (int)(freq[i] * sens) };
			Renderer::drawFilledRectangle(bar, { 255,0,0 });
		}
	}
	
	void Renderer::setPixel(int x, int y, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		//outofbounds
		if (x<=0 || x>=buffer.width || y<=0 || y>=buffer.height)
			return;

		x = buffer.width - x;
		y = buffer.height - y;

		//convert u8 colors to one u32
		uint32_t raw_color = (color.red << 16) | (color.green << 8) | (color.blue << 0);

		uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + x * bytes_per_pixel + y * buffer.pitch);
		*pixel = raw_color;
	}

	void Renderer::drawLine(Point a, Point b, const RGBColor& color)
	{
		//drawing lines using linear interpolation

		BitmapBuffer& buffer = getInstance().buffer;

		if (b.x<0 || b.x>buffer.width || b.y<0 || b.y>buffer.height || a.x<0 || a.x>buffer.width || a.y<0 || a.y>buffer.height)
			return;
		
		if (std::abs(b.x - a.x) > std::abs(b.y - a.y)) {
			if (a.x > b.x) {
				Point temp = a;
				a = b;
				b = temp;
			}

			std::vector<float> ys = interpolate(a.x, a.y, b.x, b.y);
			for (int x = a.x; x < b.x; x++) {
				setPixel(x, (int)(0.5 + ys[x - a.x]), color);
			}
		}
		else {
			if (a.y > b.y) {
				Point temp = a;
				a = b;
				b = temp;
			}

			std::vector<float> xs = interpolate(a.y, a.x, b.y, b.x);
			for (int y = a.y; y < b.y; y++) {
				setPixel((int)(0.5 + xs[y - a.y]), y, color);
			}
		}
	}

	void Renderer::drawTriangle(Point a, Point b, Point c, const RGBColor& color){
		drawLine(a, b, color);
		drawLine(b, c, color);
		drawLine(c, a, color);
	}

	void Renderer::drawFilledTriangle(haris::triangle tri, const RGBColor& color) {

		// Sort the points so that y0 <= y1 <= y2
		if (tri.p[1].coordinates.y < tri.p[0].coordinates.y) {
			std::swap(tri.p[1], tri.p[0]);
		}
		if (tri.p[2].coordinates.y < tri.p[0].coordinates.y) {
			std::swap(tri.p[0], tri.p[2]);
		}
		if (tri.p[2].coordinates.y < tri.p[1].coordinates.y) {
			std::swap(tri.p[1], tri.p[2]);
		}

		std::vector<float> xab = interpolate(tri.p[0].coordinates.y, tri.p[0].coordinates.x, tri.p[1].coordinates.y, tri.p[1].coordinates.x);
		std::vector<float> zab = interpolate(tri.p[0].coordinates.y, 1 / tri.p[0].coordinates.z, tri.p[1].coordinates.y, 1 / tri.p[1].coordinates.z);
		std::vector<float> xbc = interpolate(tri.p[1].coordinates.y, tri.p[1].coordinates.x, tri.p[2].coordinates.y, tri.p[2].coordinates.x);
		std::vector<float> zbc = interpolate(tri.p[1].coordinates.y, 1 / tri.p[1].coordinates.z, tri.p[2].coordinates.y, 1 / tri.p[2].coordinates.z);
		std::vector<float> xac = interpolate(tri.p[0].coordinates.y, tri.p[0].coordinates.x, tri.p[2].coordinates.y, tri.p[2].coordinates.x);
		std::vector<float> zac = interpolate(tri.p[0].coordinates.y, 1 / tri.p[0].coordinates.z, tri.p[2].coordinates.y, 1 / tri.p[2].coordinates.z);

		int numPixels = xab.size() + xbc.size() - 1;
		std::vector<float> xabc;
		xabc.reserve(numPixels);
		std::vector<float> zabc;
		zabc.reserve(numPixels);
		std::vector<float> habc;
		habc.reserve(numPixels);

		xab.pop_back();
		xab.insert(xab.end(), xbc.begin(), xbc.end());
		xabc = xab;

		zab.pop_back();
		zab.insert(zab.end(), zbc.begin(), zbc.end());
		zabc = zab;

		int m = std::floor(xabc.size() / 2);

		std::vector<float> xLeft;
		std::vector<float> xRight;
		std::vector<float> zLeft;
		std::vector<float> zRight;

		if (xac.at(m) < xabc.at(m)) {
			xLeft = xac;
			zLeft = zac;

			xRight = xabc;
			zRight = zabc;
		}
		else {
			xRight = xac;
			zRight = zac;

			xLeft = xabc;
			zLeft = zabc;
		}
		std::vector<float> hSegment;
		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = (int)tri.p[0].coordinates.y; y < (int)tri.p[2].coordinates.y - 1; y++) {
			int secondY = y - (int)tri.p[0].coordinates.y;

			int xL = (int)xLeft.at(secondY);
			int xR = (int)xRight.at(secondY);

			std::vector<float> zSegment = interpolate(xL, zLeft.at(secondY), xR, zRight.at(secondY));
			
			for (int x = xLeft.at(secondY); x < xRight.at(secondY) -1; x++) {
				float zDepth = zSegment.at(x - xL);
				if (zDepth > pDepthBuffer[x * screenHeight + y])	//Checking zBuffer
				{
					pDepthBuffer[x * screenHeight + y] = zDepth;
					setPixel(x, y, color);
				}
			}
		}
	}

	void Renderer::drawShadedTriangle(haris::triangle tri, const RGBColor& color) {
		// Sort the points so that y0 <= y1 <= y2
		if (tri.p[1].coordinates.y < tri.p[0].coordinates.y) {
			std::swap(tri.p[1], tri.p[0]);
		}
		if (tri.p[2].coordinates.y < tri.p[0].coordinates.y) {
			std::swap(tri.p[0], tri.p[2]);
		}
		if (tri.p[2].coordinates.y < tri.p[1].coordinates.y) {
			std::swap(tri.p[1], tri.p[2]);
		}

		std::vector<float> xab = interpolate(tri.p[0].coordinates.y, tri.p[0].coordinates.x, tri.p[1].coordinates.y, tri.p[1].coordinates.x);
		std::vector<float> zab = interpolate(tri.p[0].coordinates.y, 1 / tri.p[0].coordinates.z, tri.p[1].coordinates.y, 1 / tri.p[1].coordinates.z);
		std::vector<float> xbc = interpolate(tri.p[1].coordinates.y, tri.p[1].coordinates.x, tri.p[2].coordinates.y, tri.p[2].coordinates.x);
		std::vector<float> zbc = interpolate(tri.p[1].coordinates.y, 1 / tri.p[1].coordinates.z, tri.p[2].coordinates.y, 1 / tri.p[2].coordinates.z);
		std::vector<float> xac = interpolate(tri.p[0].coordinates.y, tri.p[0].coordinates.x, tri.p[2].coordinates.y, tri.p[2].coordinates.x);
		std::vector<float> zac = interpolate(tri.p[0].coordinates.y, 1 / tri.p[0].coordinates.z, tri.p[2].coordinates.y, 1 / tri.p[2].coordinates.z);
		std::vector<float> hab = interpolate(tri.p[0].coordinates.y, tri.p[0].h, tri.p[1].coordinates.y, tri.p[1].h);
		std::vector<float> hbc = interpolate(tri.p[1].coordinates.y, tri.p[1].h, tri.p[2].coordinates.y, tri.p[2].h);
		std::vector<float> hac = interpolate(tri.p[0].coordinates.y, tri.p[2].h, tri.p[2].coordinates.y, tri.p[2].h);

		int numPixels = xab.size() + xbc.size() - 1;
		std::vector<float> xabc;
		xabc.reserve(numPixels);
		std::vector<float> zabc;
		zabc.reserve(numPixels);
		std::vector<float> habc;
		habc.reserve(numPixels);

		hab.pop_back();
		hab.insert(hab.end(), hbc.begin(), hbc.end());
		habc = hab;
		xab.pop_back();
		xab.insert(xab.end(), xbc.begin(), xbc.end());
		xabc = xab;

		zab.pop_back();
		zab.insert(zab.end(), zbc.begin(), zbc.end());
		zabc = zab;

		int m = std::floor(xabc.size() / 2);

		std::vector<float> xLeft;
		std::vector<float> xRight;
		std::vector<float> zLeft;
		std::vector<float> zRight;
		std::vector<float> hLeft;
		std::vector<float> hRight;

		if (xac.at(m) < xabc.at(m)) {
			xLeft = xac;
			zLeft = zac;

			xRight = xabc;
			zRight = zabc;

			hLeft = hac;
			hRight = habc;
		}
		else {
			xRight = xac;
			zRight = zac;

			xLeft = xabc;
			zLeft = zabc;

			hRight = hac;
			hLeft = habc;
		}
		
		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = (int)tri.p[0].coordinates.y; y < (int)tri.p[2].coordinates.y - 1; y++) {
			int secondY = y - (int)tri.p[0].coordinates.y;

			int xL = (int)xLeft.at(secondY);
			int xR = (int)xRight.at(secondY);

			std::vector<float> zSegment = interpolate(xL, zLeft.at(secondY), xR, zRight.at(secondY));

			std::vector<float> hSegment = interpolate(xL, hLeft.at(secondY), xR, hRight.at(secondY));
			
			for (int x = xLeft.at(secondY); x < xRight.at(secondY) -1; x++) {
				
				float zDepth = zSegment.at(x - xL);
				if (zDepth > pDepthBuffer[x * screenHeight + y])	//Checking zBuffer
				{
					pDepthBuffer[x * screenHeight + y] = zDepth;
					float currentH = hSegment.at(x - xL);

					RGBColor shadedColor = { (uint8_t)(color.red * currentH), (uint8_t)(color.green * currentH), (uint8_t)(color.blue * currentH) };
					setPixel(x, y, shadedColor);
				}				
			}
		}
	}

	void Renderer::drawFilledRectangle(const Rect& rect, const RGBColor& color) 
	{
		BitmapBuffer& buffer = getInstance().buffer;

		int minX = rect.x;
		int minY = rect.y;
		int maxX = rect.x + rect.width;
		int maxY = rect.y + rect.height;

		//outofbounds
		if (minX < 0) minX = 0;
		if (minY < 0) minY = 0;
		if (maxX > buffer.width) maxX = buffer.width;
		if (maxY > buffer.height) maxY = buffer.height;

		//convert u8 colors to one u32
		uint32_t raw_color = (color.red << 16) | (color.green << 8) | (color.blue << 0);

		uint8_t* row = (uint8_t*)buffer.memory + minX * bytes_per_pixel + minY * buffer.pitch;
		for (int y = minY; y < maxY; y++) {
			uint32_t* pixel = (uint32_t*)row;
			for (int x = minX; x < maxX; x++) {
				*pixel++ = raw_color;
			}
			row += buffer.pitch;
		}
	}

	void Renderer::getWindowDimensions(int* outWidth, int* outHeight) {
		RECT clientRect;
		GetClientRect(getInstance().windowHandle, &clientRect);

		*outWidth = clientRect.right - clientRect.left;
		*outHeight = clientRect.bottom - clientRect.top;
	}

	void Renderer::resizeFrameBuffer(int width, int height) {
		BitmapBuffer& buffer = getInstance().buffer;

		if (buffer.memory) {
			VirtualFree(buffer.memory, 0, MEM_RELEASE);
		}

		buffer.width = width;
		buffer.height = height;

		buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
		buffer.info.bmiHeader.biWidth = buffer.width;
		buffer.info.bmiHeader.biHeight = -(buffer.height);
		buffer.info.bmiHeader.biPlanes = 1;
		buffer.info.bmiHeader.biBitCount = 32;
		buffer.info.bmiHeader.biCompression = BI_RGB;

		int bufferMemorySize = buffer.width * buffer.height * bytes_per_pixel;
		buffer.memory = VirtualAlloc(0, bufferMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		buffer.pitch = buffer.width * bytes_per_pixel;
	}

	void Renderer::coppyBufferToWindow(HDC deviceContext, int windowWidth, int windowHeight) {
		BitmapBuffer& buffer = getInstance().buffer;

		StretchDIBits(
			deviceContext,
			0, 0, windowWidth, windowHeight,
			0, 0, buffer.width, buffer.height,
			buffer.memory,
			&(buffer.info),
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}

	void Renderer::clear() {
		BitmapBuffer& buffer = getInstance().buffer;
		drawFilledRectangle({ 0, 0, buffer.width, buffer.height }, getInstance().clearColor);
	}
}