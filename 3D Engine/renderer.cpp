#define NOMINMAX
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
	using std::min;
	using std::max;

	double* pDepthBuffer = nullptr;

	mat4x4 matProj;

	std::list<mesh> myMeshes;

	Camera myCamera;

	float myL, myR;
	float* freq;
	float sensitivity = 15.0f;

	int screenWidth, screenHeight;


	void Renderer::moveCamera(float deltaTime) {
		float x = 0;
		float y = 0;
		float z = 0;
		float horizontalYaw = 0;
		float verticalYaw = 0;
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

		if (haris::Input::isKeyPressed(H_RIGHT))
			horizontalYaw = 1;
		else if (haris::Input::isKeyPressed(H_LEFT))
			horizontalYaw = -1;

		if (haris::Input::isKeyPressed(H_UP))
			verticalYaw = 1;
		else if (haris::Input::isKeyPressed(H_DOWN))
			verticalYaw = -1;


		

		haris::Input::Position mousePosition = haris::Input::getMousePosition();
		//haris::Input::setMousePosition({ 0, 0 });
		float lookSpeed = 0.3f;
		int rotX = (int)(lookSpeed * deltaTime * float(screenWidth / 2 - mousePosition.x));
		int rotY = (int)(lookSpeed * deltaTime * float(screenHeight / 2 - mousePosition.y));

		vec3d position = { x * myCamera.moveSpeed * deltaTime, y * myCamera.moveSpeed * deltaTime, z * myCamera.moveSpeed * deltaTime };
		myCamera.vCamera = Vector_Add(myCamera.vCamera, position);
		myCamera.horizontalYaw += horizontalYaw * myCamera.lookSpeed * deltaTime;
		myCamera.verticalYaw += verticalYaw * myCamera.lookSpeed * deltaTime;
	}
	void Renderer::RenderScene(float theta, float delta, float vol_l, float vol_r, float* freqD) {
		myL = vol_l;
		myR = vol_r;
		freq = freqD;
		moveCamera(delta);

		vec3d vUp = { 0,1,0 };
		vec3d vTarget = { 0, 0, 1 };

		mat4x4 matCameraRotX = Matrix_MakeRotationX(myCamera.verticalYaw);
		mat4x4 matCameraRotY = Matrix_MakeRotationY(myCamera.horizontalYaw);
		myCamera.vLookDir = Matrix_MultiplyVector(matCameraRotX, vTarget);
		myCamera.vLookDir = Matrix_MultiplyVector(matCameraRotY, myCamera.vLookDir);
		vTarget = Vector_Add(myCamera.vCamera, myCamera.vLookDir);

		mat4x4 matCamera = Matrix_PointAt(myCamera.vCamera, vTarget, vUp);

		mat4x4 matView = Matrix_QuickInverse(matCamera);

		//display3DFrequencyBars(theta);
		for (auto& m : myMeshes) {
			draw3dMesh(m, 0, matView);
		}
		
		//display2DFrequencyBars();
		// 
		//Clear depth buffer
		for (int i = 0; i < screenHeight * screenWidth; i++) {
			pDepthBuffer[i] = 0;
		}
	}

	void Renderer::display2DFrequencyBars() {
		int numBars = 100;
		int barWidth = 5;
		int sens = 10;
		int startX = 100;

		for (int i = 0; i < numBars; i++) {
			Rect bar = { startX + (barWidth * i), 300, barWidth, (int)(freq[i] * sens) };
			Renderer::fillRectangle(bar, {255,0,0});
		}
	}

	void Renderer::display3DFrequencyBars(float theta) {
		int numBars = 100;
		float barWidth = 0.1f;
		int sens = 1;
		int startX = 100;

		mesh grandMesh;
		grandMesh.loadFromObjectFile("Cube.txt");

		for (int b = 0; b < numBars; b++) {
			mat4x4 matRotZ = Matrix_MakeRotationZ(theta);
			mat4x4 matRotX = Matrix_MakeRotationY(theta / 2);

			mat4x4 matTrans = Matrix_MakeTranslation(-20 + ((barWidth*5) * b), 0.0f, 50.0f);

			float volStuff = 1.0f + (myL * sensitivity);
			mat4x4 matScale = Matrix_MakeScale(barWidth, (freq[b] * sens), barWidth*10);
			mat4x4 matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
			matWorld = Matrix_MultiplyMatrix(matWorld, matScale);
			matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

			vec3d vUp = { 0,1,0 };
			vec3d vTarget = { 0, 0, 1 };

			mat4x4 matCameraRot = Matrix_MakeRotationY(myCamera.horizontalYaw);
			myCamera.vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
			vTarget = Vector_Add(myCamera.vCamera, myCamera.vLookDir);

			mat4x4 matCamera = Matrix_PointAt(myCamera.vCamera, vTarget, vUp);

			mat4x4 matView = Matrix_QuickInverse(matCamera);

			std::vector<triangle> vecTrianglesToRaster;

			mesh myMesh = grandMesh;

			//draw triangles
			for (auto tri : myMesh.tris) {
				triangle triProjected, triTransformed, triViewed;

				triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
				triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
				triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

				vec3d normal, line1, line2;
				line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
				line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
				normal = Vector_CrossProduct(line1, line2);
				normal = Vector_Normalise(normal);

				vec3d vCameraRay = Vector_Sub(triTransformed.p[0], myCamera.vCamera);

				if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
				{
					triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
					triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
					triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

					int nClippedTriangles = 0;
					triangle clipped[2];
					nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

					for (int n = 0; n < nClippedTriangles; n++)
					{
						//Apply projection matrix
						triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
						triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
						triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

						//Normalize
						triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
						triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
						triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

						vec3d vOffsetView = { 1,1,0 };

						triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
						triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
						triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

						float myScale = 799.0f;
						triProjected.p[0].x *= 0.5f * myScale; triProjected.p[0].y *= 0.5f * myScale;
						triProjected.p[1].x *= 0.5f * myScale; triProjected.p[1].y *= 0.5f * myScale;
						triProjected.p[2].x *= 0.5f * myScale; triProjected.p[2].y *= 0.5f * myScale;

						triProjected.fillColor = tri.fillColor;
						triProjected.outlineColor = tri.outlineColor;

						vecTrianglesToRaster.push_back(triProjected);
					}
				}
			}

			// Sort triangles from back to front
			std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
				{
					float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
					float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
					return z1 > z2;
				});
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

					//drawFilledTriangle({ (int)t.p[0].x, (int)t.p[0].y }, { (int)t.p[1].x, (int)t.p[1].y }, { (int)t.p[2].x, (int)t.p[2].y }, { 0,255,0 });

					//Wireframe
					drawTriangle({ (int)t.p[0].x, (int)t.p[0].y },
						{ (int)t.p[1].x, (int)t.p[1].y },
						{ (int)t.p[2].x, (int)t.p[2].y }, {255, 0, 0});
				}
			}
		}
	}
	
	void Renderer::draw3dMesh(mesh myMesh, float theta, mat4x4 matView) {

		mat4x4 matRotX = Matrix_MakeRotationX(myMesh.rotation.x);
		mat4x4 matRotY = Matrix_MakeRotationZ(myMesh.rotation.y);
		mat4x4 matRotZ = Matrix_MakeRotationZ(myMesh.rotation.z);

		
		mat4x4 matScale;
		if (myMesh.linkVolume) {
			float volStuff = 1.0f + (myL * sensitivity);
			matScale = Matrix_MakeScale(myMesh.scale.x + volStuff, myMesh.scale.y + volStuff, myMesh.scale.z + volStuff);
		}
		else {
			matScale = Matrix_MakeScale(myMesh.scale.x, myMesh.scale.y, myMesh.scale.z);
		}
		
		mat4x4 matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
		mat4x4 matTrans = Matrix_MakeTranslation(myMesh.coordinates.x, myMesh.coordinates.y, myMesh.coordinates.z);

		matWorld = Matrix_MultiplyMatrix(matWorld, matScale);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		std::vector<triangle> vecTrianglesToRaster;

		//draw triangles
		for (auto tri : myMesh.tris) {
			triangle triProjected, triTransformed, triViewed;

			triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
			triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
			triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

			vec3d normal, normalisedNormal, line1, line2;
			line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
			normal = Vector_CrossProduct(line1, line2);
			normalisedNormal = Vector_Normalise(normal);

			vec3d vCameraRay = Vector_Sub(triTransformed.p[0], myCamera.vCamera);

			if(Vector_DotProduct(normalisedNormal, vCameraRay) < 0.0f)
			{
				triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
				triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
				triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

				int nClippedTriangles = 0;
				triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				for (int n = 0; n < nClippedTriangles; n++)
				{
					//Apply projection matrix
					triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);

					//Normalize
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					vec3d vOffsetView = { 1,1,0 };

					triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
					triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
					triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

					float myScale = 799.0f;
					triProjected.p[0].x *= 0.5f * myScale; triProjected.p[0].y *= 0.5f * myScale; triProjected.p[0].z *= 0.5f * myScale;
					triProjected.p[1].x *= 0.5f * myScale; triProjected.p[1].y *= 0.5f * myScale; triProjected.p[1].z *= 0.5f * myScale;
					triProjected.p[2].x *= 0.5f * myScale; triProjected.p[2].y *= 0.5f * myScale; triProjected.p[2].z *= 0.5f * myScale;

					triProjected.fillColor = tri.fillColor;
					triProjected.outlineColor = tri.outlineColor;

					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}

		// Sort triangles from back to front
		/*std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return z1 > z2;
			});*/

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
					drawTriangle({ (int)t.p[0].x, (int)t.p[0].y },
						{ (int)t.p[1].x, (int)t.p[1].y },
						{ (int)t.p[2].x, (int)t.p[2].y }, t.outlineColor);
				}
				if (myMesh.drawFilled) {
					drawFilledTriangle(t, t.fillColor, myMesh.drawShaded);
				}	
			}

			
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

	void Renderer::drawFilledTriangle(haris::triangle tri, const RGBColor& color, bool isShaded) {

		// Sort the points so that y0 <= y1 <= y2
		if (tri.p[1].y < tri.p[0].y) {
			std::swap(tri.p[1], tri.p[0]);
		}
		if (tri.p[2].y < tri.p[0].y) {
			std::swap(tri.p[0], tri.p[2]);
		}
		if (tri.p[2].y < tri.p[1].y) {
			std::swap(tri.p[1], tri.p[2]);
		}

		std::vector<float> xab = interpolate(tri.p[0].y, tri.p[0].x, tri.p[1].y, tri.p[1].x);
		std::vector<float> zab = interpolate(tri.p[0].y, 1/tri.p[0].z, tri.p[1].y, 1/tri.p[1].z);
		std::vector<float> xbc = interpolate(tri.p[1].y, tri.p[1].x, tri.p[2].y, tri.p[2].x);
		std::vector<float> zbc = interpolate(tri.p[1].y, 1/tri.p[1].z, tri.p[2].y, 1/tri.p[2].z);
		std::vector<float> xac = interpolate(tri.p[0].y, tri.p[0].x, tri.p[2].y, tri.p[2].x);
		std::vector<float> zac = interpolate(tri.p[0].y, 1/tri.p[0].z, tri.p[2].y, 1/tri.p[2].z);
		std::vector<float> hab;
		std::vector<float> hbc;
		std::vector<float> hac;

		int numPixels = xab.size() + xbc.size() - 1;
		std::vector<float> xabc;
		xabc.reserve(numPixels);
		std::vector<float> zabc;
		zabc.reserve(numPixels);
		std::vector<float> habc;
		habc.reserve(numPixels);

		if (isShaded) {
			hab = interpolate(tri.p[0].y, tri.h[0], tri.p[1].y, tri.h[1]);
			hbc = interpolate(tri.p[1].y, tri.h[1], tri.p[2].y, tri.h[2]);
			hac = interpolate(tri.p[0].y, tri.h[0], tri.p[2].y, tri.h[2]);
			hab.pop_back();
			hab.insert(hab.end(), hbc.begin(), hbc.end());
			habc = hab;
		}
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
		std::vector<float> hSegment;
		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = (int)tri.p[0].y; y < (int)tri.p[2].y - 1; y++) {
			int secondY = y - (int)tri.p[0].y;

			int xL = (int)xLeft.at(secondY);
			int xR = (int)xRight.at(secondY);

			std::vector<float> zSegment = interpolate(xL, zLeft.at(secondY), xR, zRight.at(secondY));

			if (isShaded) { hSegment = interpolate(xL, hLeft.at(secondY), xR, hRight.at(secondY)); }
			
			for (int x = xLeft.at(secondY); x < xRight.at(secondY) -1; x++) {
				
				float zDepth = zSegment.at(x - xL);
				if (zDepth > pDepthBuffer[x * screenHeight + y])	//Checking zBuffer
				{	
					pDepthBuffer[x * screenHeight + y] = zDepth;

					if (isShaded) {
						float currentH = hSegment.at(x - xL);
						RGBColor shadedColor = { (uint8_t)(color.red * currentH), (uint8_t)(color.green * currentH), (uint8_t)(color.blue * currentH) };
						setPixel(x, y, shadedColor);
					}
					else {
						//RGBColor grey = { 255 * zDepth, 255 * zDepth, 255 * zDepth };
						setPixel(x, y, color);
					}	
				}
			}
		}
	}

	/*void Renderer::drawShadedTriangle(Point a, Point b, Point c, RGBColor& color, float h0, float h1, float h2) {
		// Sort the points so that y0 <= y1 <= y2
		if (b.y < a.y) {
			std::swap(a, b);
			std::swap(h0, h1);
		}
		if (c.y < a.y) {
			std::swap(a, c);
			std::swap(h0, h2);
		}
		if (c.y < b.y) {
			std::swap(c, b);
			std::swap(h2, h1);
		}

		std::vector<float> xab = interpolate(a.y, a.x, b.y, b.x);
		std::vector<float> hab = interpolate(a.y, h0, b.y, h1);

		std::vector<float> xbc = interpolate(b.y, b.x, c.y, c.x);
		std::vector<float> hbc = interpolate(b.y, h1, c.y, h2);

		std::vector<float> xac = interpolate(a.y, a.x, c.y, c.x);
		std::vector<float> hac = interpolate(a.y, h0, c.y, h2);

		xab.pop_back();
		std::vector<float> xabc;
		xabc.reserve(xab.size() + xbc.size());
		xabc.insert(xabc.end(), xab.begin(), xab.end());
		xabc.insert(xabc.end(), xbc.begin(), xbc.end());

		hab.pop_back();
		std::vector<float> habc;
		habc.reserve(hab.size() + hbc.size());
		habc.insert(habc.end(), hab.begin(), hab.end());
		habc.insert(habc.end(), hbc.begin(), hbc.end());

		int m = std::floor(xabc.size() / 2);

		std::vector<float> xLeft;
		std::vector<float> xRight;

		std::vector<float> hLeft;
		std::vector<float> hRight;

		if (xac.at(m) < xabc.at(m)) {
			xLeft = xac;
			xRight = xabc;

			hLeft = hac;
			hRight = habc;
		}
		else {
			xRight = xac;
			xLeft = xabc;

			hRight = hac;
			hLeft = habc;
		}

		float xL;
		float xR;

		BitmapBuffer& buffer = getInstance().buffer;

		for (int y = a.y; y < c.y - 1; y++) {
			xL = xLeft.at(y - a.y);
			xR = xRight.at(y - a.y);

			std::vector<float> hSegment = interpolate(xL, hLeft.at(y - a.y), xR, hRight.at(y - a.y));
			for (int x = xL; x <= xR; x++) {
				float currentH = hSegment.at(x - xL);
				RGBColor shadedColor = { (uint8_t)(color.red * currentH), (uint8_t)(color.green * currentH), (uint8_t)(color.blue * currentH) };
				uint32_t raw_color = (shadedColor.red << 16) | (shadedColor.green << 8) | (shadedColor.blue << 0);
				int tempX = buffer.width - x;
				int tempY = buffer.height - y;
				if (tempX > 0 && tempX < buffer.width && tempY>0 && tempY < buffer.height) {
					uint32_t* pixel = (uint32_t*)((uint8_t*)buffer.memory + tempX * bytes_per_pixel + tempY * buffer.pitch);
					*pixel = raw_color;
				}
			}
		}
	}*/

	

	void Renderer::fillRectangle(const Rect& rect, const RGBColor& color) 
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

	//Create init function
	void Renderer::initVars() {
		screenHeight = 800;
		screenWidth = 800;

		pDepthBuffer = new double[screenHeight * screenWidth];
		for (int i = 0; i < screenHeight * screenWidth; i++) {
			pDepthBuffer[i] = 0;
		}

		matProj = Matrix_MakeProjection(90.0f, 1.0f, 0.1f, 1000.0f);

		//Cube
		mesh meshCube;
		meshCube.loadFromObjectFile("Cube.txt");
		if (meshCube.tris.size() == 0) {
			exit(0);
		}
		meshCube.randomizeTriColors();
		meshCube.coordinates = { 0, 5, 0, 0 };
		meshCube.rotation = { 0, 0, 0, 0 };
		meshCube.drawFilled = true;
		meshCube.drawShaded = true;
		meshCube.drawWireframe = false;
		myMeshes.push_back(meshCube);

		//Ground
		mesh meshFlatGround;
		meshFlatGround.loadFromObjectFile("FlatGround.txt");
		if (meshCube.tris.size() == 0) {
			exit(0);
		}
		meshFlatGround.randomizeTriColors();
		meshFlatGround.scale = { 1, 1, 1, 0 };
		meshFlatGround.drawFilled = true;
		meshFlatGround.drawShaded = false;
		meshFlatGround.drawWireframe = false;
		myMeshes.push_back(meshFlatGround);
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
		fillRectangle({ 0, 0, buffer.width, buffer.height }, getInstance().clearColor);
	}
}